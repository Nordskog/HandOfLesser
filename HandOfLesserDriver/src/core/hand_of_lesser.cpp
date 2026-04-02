#include "hand_of_lesser.h"
#include "HandOfLesserCommon.h"
#include <driverlog.h>
#include "src/controller/emulated_controller_driver.h"
#include "src/controller/generic_control_interface.h"
#include "src/utils/math_utils.h"
#include <nlohmann/json.hpp>
#include <src/json/types.h>
#include <algorithm>
#include <limits>

namespace HOL
{

	HandOfLesser* HandOfLesser::Current = nullptr;
	HOL::settings::HandOfLesserSettings HandOfLesser::Config;
	HOL::state::TrackingState HandOfLesser::Tracking;
	HOL::state::RuntimeState HandOfLesser::Runtime;

	HandOfLesser::HandOfLesser()
	{
	}

	void HandOfLesser::init()
	{
		this->mActive = true;
		HandOfLesser::Current = this;

		// Initialize transport as server (creates named pipe and waits for client)
		this->mTransport.init(PipeRole::Server, R"(\\.\pipe\HandOfLesser)");

		my_pose_update_thread_ = std::thread(&HandOfLesser::ReceiveDataThread, this);
	}

	void HandOfLesser::ReceiveDataThread()
	{
		DriverLog("ReceiveDataThread() start, active: %s", this->mActive ? "true" : "false");

		// Wait for client connection (app to connect to our pipe)
		DriverLog("Waiting for app to connect...");
		while (this->mActive && !this->mTransport.isConnected())
		{
			if (this->mTransport.waitForConnection(1000))
			{
				DriverLog("App connected!");

				// Send initialization message
				DriverInitializedPacket initPacket;
				this->mTransport.send((char*)&initPacket, sizeof(initPacket));
				DriverLog("Sent DriverInitialized packet");
				break;
			}
		}

		while (this->mActive)
		{
			HOL::NativePacket* rawPacket = this->mTransport.receivePacket();
			if (rawPacket == nullptr)
			{
				// Check if disconnected
				if (!this->mTransport.isConnected())
				{
					DriverLog("App disconnected, waiting for reconnection...");
					if (this->mTransport.waitForConnection(5000))
					{
						DriverLog("App reconnected!");

						// Send initialization message again
						DriverInitializedPacket initPacket;
						this->mTransport.send((char*)&initPacket, sizeof(initPacket));
						DriverLog("Sent DriverInitialized packet");
					}
				}
				continue;
			}
			switch (rawPacket->packetType)
			{
				case HOL::NativePacketType::HandTransform: {
					HOL::HandTransformPacket* packet = (HOL::HandTransformPacket*)rawPacket;

					// Ehhh something might send garbage data and get lucky
					if (packet->side == HOL::HandSide::LeftHand
						|| packet->side == HOL::HandSide::RightHand)
					{
						mLastHandTransforms[packet->side] = *packet;
						mHasHandTransform[packet->side] = packet->valid;
					}

					if (Config.handPose.controllerMode != ControllerMode::HookedControllerMode)
					{
						if (auto hooked = getHookedController(packet->side))
						{
							hooked->UpdatePose(packet);
						}
					}

					GenericControllerInterface* controller
						= this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdatePose(packet);
						controller->SubmitPose();
					}

					updateControllerConnectionStates();

					break;
				}

				case HOL::NativePacketType::Settings: {
					HOL::SettingsPacket* packet = (HOL::SettingsPacket*)rawPacket;

					try
					{
						HOL::settings::HandOfLesserSettings oldSettings = Config;

						// Parse JSON from packet
						nlohmann::json j = nlohmann::json::parse(packet->jsonData);
						HandOfLesser::Config = j.get<HOL::settings::HandOfLesserSettings>();

						// Handle any configuration changes
						handleConfigurationChange(oldSettings);
					}
					catch (const std::exception& ex)
					{
						DriverLog("Failed to parse settings JSON: %s", ex.what());
					}

					break;
				}

				case HOL::NativePacketType::FloatInput: {
					HOL::FloatInputPacket* packet = (HOL::FloatInputPacket*)rawPacket;
					auto controller = this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdateFloatInput(packet->inputName, packet->value);
					}

					break;
				}

				case HOL::NativePacketType::BoolInput: {

					HOL::BoolInputPacket* packet = (HOL::BoolInputPacket*)rawPacket;
					auto controller = this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdateBoolInput(packet->inputName, packet->value);
					}

					break;
				}

				case HOL::NativePacketType::SkeletalInput: {
					HOL::SkeletalPacket* packet = (HOL::SkeletalPacket*)rawPacket;

					// Send to active controller (for normal skeletal input)
					auto activeController = this->GetActiveController(packet->side);
					if (activeController != nullptr)
					{
						activeController->UpdateSkeletal(packet);
					}
					else
					{
						// send to hooked controller when augmentation enabled
						if (Config.skeletal.augmentControllerSkeleton
							&& Tracking.isMultimodalEnabled)
						{
							auto hookedController = getHookedController(packet->side);
							if (hookedController != nullptr)
							{
								hookedController->UpdateSkeletal(packet);
							}
						}
					}

					break;
				}

				case HOL::NativePacketType::MultimodalPose: {
					mLastMultimodalPosePacket = *(HOL::MultimodalPosePacket*)rawPacket;

					break;
				}

				case HOL::NativePacketType::BodyTrackerPose: {
					HOL::BodyTrackerPosePacket* packet = (HOL::BodyTrackerPosePacket*)rawPacket;

					// Find the tracker for this role
					auto it = mEmulatedTrackers.find(packet->role);
					if (it != mEmulatedTrackers.end())
					{
						it->second->UpdatePose(*packet);
						it->second->SubmitPose();
					}

					break;
				}

				case HOL::NativePacketType::State: {
					HOL::StatePacket* packet = (HOL::StatePacket*)rawPacket;

					Tracking = packet->tracking;
					Runtime = packet->runtime;
					updateControllerConnectionStates();
					break;
				}

				case HOL::NativePacketType::AppInitialized: {
					DriverLog("App initialized, sending current device state");
					sendAllDeviceStates();
					sendStatus();
					break;
				}

				default: {
					// Invalid packet type!
				}
			}
		}
	}

	void HandOfLesser::handleConfigurationChange(HOL::settings::HandOfLesserSettings& oldConfig)
	{
		bool devicesChanged = false;

		// Add or remove ( or enable/disable ) emulated controllers on controller mode change
		if (Config.handPose.controllerMode != oldConfig.handPose.controllerMode)
		{
			devicesChanged = true;
			if (Config.handPose.controllerMode == ControllerMode::EmulateControllerMode)
			{
				addEmulatedControllers();
			}
			else if (oldConfig.handPose.controllerMode == ControllerMode::EmulateControllerMode)
			{
				removeEmulatedControllers();
			}
		}

		if ((Config.handPose.emulatedControllerProfile
				 != oldConfig.handPose.emulatedControllerProfile
			 || Config.skeletal.trackingLevel != oldConfig.skeletal.trackingLevel)
			&& Config.handPose.controllerMode == ControllerMode::EmulateControllerMode)
		{
			devicesChanged = true;
			removeEmulatedControllers();
			addEmulatedControllers();
		}

		// Handle body tracker changes
		bool trackersEnabledChanged
			= Config.bodyTrackers.enableBodyTrackers != oldConfig.bodyTrackers.enableBodyTrackers;
		bool anyTrackerSettingChanged
			= Config.bodyTrackers.enableHips != oldConfig.bodyTrackers.enableHips
			  || Config.bodyTrackers.enableChest != oldConfig.bodyTrackers.enableChest
			  || Config.bodyTrackers.enableRightUpperArm
					 != oldConfig.bodyTrackers.enableRightUpperArm
			  || Config.bodyTrackers.enableRightLowerArm
					 != oldConfig.bodyTrackers.enableRightLowerArm;

		if (trackersEnabledChanged || anyTrackerSettingChanged)
		{
			devicesChanged = true;
			if (Config.bodyTrackers.enableBodyTrackers)
			{
				addEmulatedTrackers();
			}
			else
			{
				removeEmulatedTrackers();
			}
		}

		refreshPreferredHookedControllers();

		// Update logic wouldn't normally run unless in certain modes, so force upon config change.
		updateControllerConnectionStates(true);
		updateTrackerConnectionStates();
		updateShadowTrackerStates();

		// Only notify app if we actually changed the device list
		if (devicesChanged)
		{
			sendStatus();
		}
	}

	void HandOfLesser::addEmulatedControllers()
	{
		HOL::EmulatedControllerProfile profile = Config.handPose.emulatedControllerProfile;
		vr::EVRSkeletalTrackingLevel trackingLevel = getRequestedSkeletalTrackingLevel();
		HOL::EmulatedControllerVariant variant
			= HOL::getEmulatedControllerVariant(profile, trackingLevel);

		for (int i = 0; i < HOL::HandSide_MAX; i++)
		{
			// Existing controllers reomved on change, so if already there then should be right.
			if (this->mEmulatedControllers[i])
			{
				continue;
			}

			// Emulated controllers are cached by controller variant so the driver can switch between
			// profile/tracking-level combinations without recreating devices each time.
			auto& controllerSlot = this->mAllEmulatedControllers[variant][i];
			if (!controllerSlot)
			{
				controllerSlot = std::make_unique<EmulatedControllerDriver>(
					i == HOL::HandSide::LeftHand ? vr::TrackedControllerRole_LeftHand
												 : vr::TrackedControllerRole_RightHand,
					profile,
					trackingLevel);
				EmulatedControllerDriver* controller = controllerSlot.get();

				if (!vr::VRServerDriverHost()->TrackedDeviceAdded(
						controller->MyGetSerialNumber().c_str(),
						vr::TrackedDeviceClass_Controller,
						controller))
				{
					DriverLog("Failed to create %s controller device!",
							  (i == 0 ? "left" : "right"));
					controllerSlot.reset();
					continue;
				}
			}

			this->mEmulatedControllers[i] = controllerSlot.get();
		}

		updateControllerConnectionStates();
	}

	// Removes from active slots and disconnects
	void HandOfLesser::removeEmulatedControllers()
	{
		for (int i = 0; i < HOL::HandSide_MAX; i++)
		{
			if (!this->mEmulatedControllers[i])
			{
				continue;
			}

			this->mEmulatedControllers[i]->setConnectedState(false);
			this->mEmulatedControllers[i] = nullptr;
		}
	}

	// Actually nukes
	void HandOfLesser::destroyEmulatedControllers()
	{
		removeEmulatedControllers();

		for (int variant = 0; variant < HOL::EmulatedControllerVariant_MAX; variant++)
		{
			for (int side = 0; side < HOL::HandSide_MAX; side++)
			{
				if (this->mAllEmulatedControllers[variant][side])
				{
					this->mAllEmulatedControllers[variant][side]->setConnectedState(false);
					this->mAllEmulatedControllers[variant][side].reset();
				}
			}
		}
	}

	void HandOfLesser::addEmulatedTrackers()
	{
		// Add trackers for each enabled role
		for (int i = 0; i < static_cast<int>(BodyTrackerRole::TrackerRole_MAX); i++)
		{
			BodyTrackerRole role = static_cast<BodyTrackerRole>(i);

			// Check if this tracker should be enabled (master switch + individual enable)
			bool shouldBeEnabled = Config.bodyTrackers.enableBodyTrackers;
			if (shouldBeEnabled)
			{
				switch (role)
				{
					case BodyTrackerRole::Hips:
						shouldBeEnabled = Config.bodyTrackers.enableHips;
						break;
					case BodyTrackerRole::Chest:
						shouldBeEnabled = Config.bodyTrackers.enableChest;
						break;
					case BodyTrackerRole::LeftUpperArm:
						shouldBeEnabled = Config.bodyTrackers.enableLeftUpperArm;
						break;
					case BodyTrackerRole::LeftLowerArm:
						shouldBeEnabled = Config.bodyTrackers.enableLeftLowerArm;
						break;
					case BodyTrackerRole::RightUpperArm:
						shouldBeEnabled = Config.bodyTrackers.enableRightUpperArm;
						break;
					case BodyTrackerRole::RightLowerArm:
						shouldBeEnabled = Config.bodyTrackers.enableRightLowerArm;
						break;
					default:
						shouldBeEnabled = false;
						break;
				}
			}

			if (!shouldBeEnabled)
				continue;

			// Check if tracker already exists
			if (mEmulatedTrackers.find(role) != mEmulatedTrackers.end())
			{
				// Tracker already exists, just ensure it's connected
				mEmulatedTrackers[role]->setConnectedState(true);
			}
			else
			{
				// Create new tracker
				auto tracker = std::make_unique<EmulatedTrackerDriver>(role);

				// Register with SteamVR
				if (vr::VRServerDriverHost()->TrackedDeviceAdded(
						tracker->MyGetSerialNumber().c_str(),
						vr::TrackedDeviceClass_GenericTracker,
						tracker.get()))
				{
					mEmulatedTrackers[role] = std::move(tracker);
					DriverLog("Added body tracker: %s", bodyTrackerRoleToString(role));
				}
				else
				{
					DriverLog("Failed to add body tracker: %s", bodyTrackerRoleToString(role));
				}
			}
		}

		updateTrackerConnectionStates();
	}

	void HandOfLesser::removeEmulatedTrackers()
	{
		// Disconnect all trackers
		for (auto& pair : mEmulatedTrackers)
		{
			pair.second->setConnectedState(false);
		}
	}

	void HandOfLesser::updateTrackerConnectionStates()
	{
		// Update each tracker's connection state based on settings
		for (auto& pair : mEmulatedTrackers)
		{
			BodyTrackerRole role = pair.first;
			bool shouldBeConnected = Config.bodyTrackers.enableBodyTrackers;

			if (shouldBeConnected)
			{
				switch (role)
				{
					case BodyTrackerRole::Hips:
						shouldBeConnected = Config.bodyTrackers.enableHips;
						break;
					case BodyTrackerRole::Chest:
						shouldBeConnected = Config.bodyTrackers.enableChest;
						break;
					case BodyTrackerRole::LeftUpperArm:
						shouldBeConnected = Config.bodyTrackers.enableLeftUpperArm;
						break;
					case BodyTrackerRole::LeftLowerArm:
						shouldBeConnected = Config.bodyTrackers.enableLeftLowerArm;
						break;
					case BodyTrackerRole::RightUpperArm:
						shouldBeConnected = Config.bodyTrackers.enableRightUpperArm;
						break;
					case BodyTrackerRole::RightLowerArm:
						shouldBeConnected = Config.bodyTrackers.enableRightLowerArm;
						break;
					default:
						shouldBeConnected = false;
						break;
				}
			}

			pair.second->setConnectedState(shouldBeConnected);
		}
	}

	void HandOfLesser::updateControllerConnectionStates(bool forceUpdate)
	{
		const bool emulationMode
			= Config.handPose.controllerMode == ControllerMode::EmulateControllerMode;
		const bool hookedMode
			= Config.handPose.controllerMode == ControllerMode::HookedControllerMode;

		if (hookedMode && forceUpdate)
		{
			for (auto& controllerContainer : mHookedControllers)
			{
				HookedController* hooked = controllerContainer.get();

				if (hooked->mDeviceClass != vr::TrackedDeviceClass_Controller)
				{
					continue;
				}

				if (hooked->isActingAsTracker())
				{
					continue;
				}

				// We don't want to spam disconnect signals, but we do want to force the hooked
				// controllers back through a clean reconnect path after a mode/settings change.
				// If the controllers are still active their next real pose submission will bring
				// them back immediately, so this is safe as a one-off reset.
				if (forceUpdate)
				{
					hooked->sendDisconnectState();
				}
			}
		}

		for (int i = 0; i < HOL::HandSide_MAX; ++i)
		{
			auto side = static_cast<HOL::HandSide>(i);

			if (forceUpdate || emulationMode)
			{
				bool handTrackingPrimary = emulationMode && isHandTrackingPrimary(side);

				// TODO: Add bool to determine if we should suppress existing controllers
				for (auto* hooked : getHookedControllers(side))
				{
					// This will set suppressed state and a disconnect event depending on the
					// existing suppression state, meaning it may not trigger if we've been
					// submitting poses on its behalf already.
					// Don't unsuppress if the controller is acting as a tracker.
					if (!hooked->isActingAsTracker())
					{
						hooked->setSuppressed(handTrackingPrimary);
					}
				}

				// Just sets an internal bool to enabled so it accepts data,
				// and we can tell whether or not it was active when we disable it.
				if (auto emulated = getEmulatedController(side))
				{
					emulated->setConnectedState(handTrackingPrimary);
				}


				// Ensure inactive controllers are disconnected
				HOL::EmulatedControllerVariant activeVariant = HOL::getEmulatedControllerVariant(
					Config.handPose.emulatedControllerProfile, getRequestedSkeletalTrackingLevel());
				for (int variant = 0; variant < HOL::EmulatedControllerVariant_MAX; variant++)
				{
					if (variant == activeVariant)
					{
						continue;
					}

					auto& inactive = mAllEmulatedControllers[variant][i];
					if (inactive)
					{
						inactive->setConnectedState(false);
					}
				}
			}
		}
	}

	HookedController*
	HandOfLesser::addHookedController(uint32_t id,
									  vr::IVRServerDriverHost* host,
									  vr::ITrackedDeviceServerDriver* driver,
									  vr::PropertyContainerHandle_t propertyContainer)
	{

		this->mHookedControllers.push_back(std::make_unique<HookedController>(
			id, HandSide::HandSide_MAX, host, driver, propertyContainer));

		HookedController* newController = this->mHookedControllers.back().get();

		return newController;
	}

	// We don't bother removing devices when they're deactivated at the moment, since we
	// may went to continue controlling them. This may mean they can be activated again.
	// We can't identify them until they have been fully activated, so once they have been
	// fully activated and populated, check for duplicate serials and nuke the oldest instance.
	void HandOfLesser::removeDuplicateDevices()
	{
		std::unordered_map<std::string, int> existingSerials;

		// count duplicates
		for (auto& controllerContainer : mHookedControllers)
		{
			existingSerials[controllerContainer->serial]++;
		}

		// Starting from oldest, delete while duplicate count > 1
		auto it = mHookedControllers.begin();
		while (it != mHookedControllers.end())
		{
			if (existingSerials[it->get()->serial] > 1)
			{
				DriverLog("Removing duplicate device with serial: %s", it->get()->serial.c_str());

				it->reset();
				existingSerials[it->get()->serial]--;
				it = mHookedControllers.erase(it);
			}
			else
			{
				it++;
			}
		}

		refreshPreferredHookedControllers();
	}

	bool HandOfLesser::shouldPossess(uint32_t deviceId)
	{
		if (HandOfLesser::Current->Config.handPose.controllerMode
			== ControllerMode::HookedControllerMode)
		{
			return shouldPossess(getHookedControllerByDeviceId(deviceId));
		}

		return false;
	}

	bool HandOfLesser::shouldPossess(HookedController* controller)
	{
		if (HandOfLesser::Current->Config.handPose.controllerMode
			!= ControllerMode::HookedControllerMode)
		{
			return false;
		}

		// Only the selected controller for a side should ever be possessed.
		if (controller == nullptr || controller != getHookedController(controller->getSide()))
		{
			return false;
		}

		if (!shouldUseHandTracking(controller))
		{
			return false;
		}

		return true;
	}

	bool HandOfLesser::shouldEmulateControllers()
	{
		return HandOfLesser::Current->Config.handPose.controllerMode
			   == ControllerMode::EmulateControllerMode;
	}

	bool HandOfLesser::shouldUseHandTracking(HookedController* controller)
	{
		if (controller == nullptr)
		{
			return false;
		}

		if (controller->mDeviceClass != vr::TrackedDeviceClass_Controller)
		{
			return false;
		}

		const auto& trackingState = HOL::HandOfLesser::Tracking;

		if (trackingState.isMultimodalEnabled)
		{
			// If controller is configured to always act as tracker (alsoWhenHeld),
			// the real controller will never come back, so keep using hand tracking
			auto it = Config.deviceSettings.devices.find(controller->serial);
			if (it != Config.deviceSettings.devices.end() && it->second.actAsTracker
				&& it->second.alsoWhenHeld)
			{
				return true; // Always use hand tracking
			}

			bool shouldPossess = !controller->isHeld();
			return shouldPossess;
		}

		// Recovery controller is the first pair of real controllers we encountered.
		// If a driver provides hand-tracking controllers we may be hooking 
		// those instead of the real controllers, but still want to use the real
		// controls to determine whether or not we should stop hand tracking entirely.
		HookedController* recoveryController = getRecoveryHookedController(controller->getSide());
		if (recoveryController == nullptr)
		{
			recoveryController = controller;
		}

		bool canPoss = controller->canPossess();
		bool recoveryPoseValid = recoveryController->mLastOriginalPoseValid;
		if (!recoveryPoseValid && canPoss)
		{
			controller->mValidWhileOriginalInvalid = true;
		}

		if (recoveryPoseValid)
		{
			controller->mValidWhileOriginalInvalid = false;
		}

		bool originalSubmitStale = recoveryController->framesSinceLastPoseUpdate > 30;

		bool shouldPossess
			= controller->mLastTransformPacket.tracked
			  || (canPoss && (!recoveryPoseValid || originalSubmitStale))
			  || controller->mValidWhileOriginalInvalid;

		return shouldPossess;
	}

	bool HandOfLesser::shouldSuppressHookedController(HookedController* controller)
	{
		if (controller == nullptr)
		{
			return false;
		}

		if (Config.handPose.controllerMode != ControllerMode::HookedControllerMode)
		{
			return false;
		}

		if (controller->mDeviceClass != vr::TrackedDeviceClass_Controller)
		{
			return false;
		}

		if (controller->isActingAsTracker())
		{
			return false;
		}

		HandSide side = controller->getSide();
		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return false;
		}

		if (!isHandTrackingPrimary(side))
		{
			return false;
		}

		// Once a side is hand-tracking-primary, only the selected hooked controller for that
		// side stays live. Every other same-side controller should be hidden until native
		// control takes priority again.
		return controller != getHookedController(side);
	}

	bool HandOfLesser::isHandTrackingPrimary(HOL::HandSide side)
	{
		HookedController* hooked = getHookedController(side);
		if (hooked != nullptr)
		{
			return shouldUseHandTracking(hooked);
		}

		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return false;
		}

		if (!mHasHandTransform[side])
		{
			return false;
		}

		const HOL::HandTransformPacket& packet = mLastHandTransforms[side];
		return packet.valid && packet.tracked;
	}

	EmulatedControllerDriver* HandOfLesser::getEmulatedController(HOL::HandSide side)
	{
		return this->mEmulatedControllers[(int)side];
	}

	bool HandOfLesser::isEmulatedController(vr::ITrackedDeviceServerDriver* driver)
	{
		for (auto& variantControllers : mAllEmulatedControllers)
		{
			for (auto& controllerContainer : variantControllers)
			{
				EmulatedControllerDriver* controller = controllerContainer.get();
				if (controller == driver)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool HandOfLesser::isEmulatedTracker(vr::ITrackedDeviceServerDriver* driver)
	{
		for (auto& tracker : mEmulatedTrackers)
		{
			EmulatedTrackerDriver* controller = tracker.second.get();
			if (controller == driver)
			{
				return true;
			}
		}

		return false;
	}

	bool HandOfLesser::isShadowTracker(vr::ITrackedDeviceServerDriver* driver)
	{
		for (auto& pair : mShadowTrackers)
		{
			if (pair.second.get() == driver)
			{
				return true;
			}
		}
		return false;
	}

	EmulatedTrackerDriver* HandOfLesser::getOrCreateShadowTracker(HookedController* controller)
	{
		const std::string& serial = controller->serial;

		auto it = mShadowTrackers.find(serial);
		if (it != mShadowTrackers.end())
		{
			// Ensure the controller has the pointer (may have been cleared)
			controller->setShadowTracker(it->second.get());
			return it->second.get();
		}

		// Create new shadow tracker
		auto tracker = std::make_unique<EmulatedTrackerDriver>(serial);

		// Register with SteamVR
		if (vr::VRServerDriverHost()->TrackedDeviceAdded(tracker->MyGetSerialNumber().c_str(),
														 vr::TrackedDeviceClass_GenericTracker,
														 tracker.get()))
		{
			EmulatedTrackerDriver* ptr = tracker.get();
			mShadowTrackers[serial] = std::move(tracker);
			controller->setShadowTracker(ptr);
			DriverLog("Created shadow tracker for: %s", serial.c_str());
			return ptr;
		}

		DriverLog("Failed to create shadow tracker for: %s", serial.c_str());
		return nullptr;
	}

	void HandOfLesser::updateShadowTrackerState(HookedController* controller)
	{
		bool shouldAct = controller->shouldActAsTracker();
		bool isActing = controller->isActingAsTracker();

		if (shouldAct && !isActing)
		{
			// Transition: controller → tracker
			auto* shadow = getOrCreateShadowTracker(controller);
			if (shadow)
			{
				shadow->setConnectedState(true);
				controller->setSuppressed(true);
				controller->setActingAsTracker(true);
				DriverLog("Controller %s now acting as tracker", controller->serial.c_str());
			}
		}
		else if (!shouldAct && isActing)
		{
			// Transition: tracker → controller
			// Try controller pointer first, fall back to map lookup
			EmulatedTrackerDriver* shadow = controller->getShadowTracker();
			if (!shadow)
			{
				auto it = mShadowTrackers.find(controller->serial);
				if (it != mShadowTrackers.end())
				{
					shadow = it->second.get();
				}
			}
			if (shadow)
			{
				shadow->setConnectedState(false);
			}
			controller->setSuppressed(false);
			controller->setActingAsTracker(false);
			DriverLog("Controller %s no longer acting as tracker", controller->serial.c_str());
		}
	}

	void HandOfLesser::updateShadowTrackerStates()
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			updateShadowTrackerState(controllerContainer.get());
		}
	}

	// Only works if a side has been assigned
	// usually is, just not right after being activated.
	HookedController* HandOfLesser::getHookedController(HOL::HandSide side)
	{
		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return nullptr;
		}

		return mPreferredHookedControllers[side];
	}

	std::vector<HookedController*> HandOfLesser::getHookedControllers(HOL::HandSide side)
	{
		std::vector<HookedController*> controllers;

		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return controllers;
		}

		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			if (controller->mDeviceClass != vr::TrackedDeviceClass_Controller)
			{
				continue;
			}

			if (controller->getSide() != side)
			{
				continue;
			}

			controllers.push_back(controller);
		}

		return controllers;
	}

	void HandOfLesser::refreshPreferredHookedControllers()
	{
		// Preferred possessed controllers and the real-controller recovery pair both only change
		// when device availability, side assignment, or settings change, so cache them together
		// instead of rescanning the full hooked list every frame.
		for (int i = 0; i < HOL::HandSide_MAX; ++i)
		{
			HOL::HandSide side = static_cast<HOL::HandSide>(i);
			refreshPreferredHookedController(side);
			refreshRecoveryHookedController(side);
		}
	}

	void HandOfLesser::refreshPreferredHookedController(HOL::HandSide side)
	{
		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return;
		}

		mPreferredHookedControllers[side] = nullptr;

		auto controllers = getHookedControllers(side);
		if (controllers.empty())
		{
			return;
		}

		std::string preferredSerial = getPreferredHookedControllerSerial(side);
		if (!preferredSerial.empty())
		{
			// Explicit user selection always wins when that serial is currently available.
			for (HookedController* controller : controllers)
			{
				if (controller->serial == preferredSerial)
				{
					mPreferredHookedControllers[side] = controller;
					return;
				}
			}
		}

		HookedController* bestController = nullptr;
		int bestScore = std::numeric_limits<int>::lowest();

		for (HookedController* controller : controllers)
		{
			int score = getHookedControllerSelectionScore(controller);
			if (bestController == nullptr || score > bestScore
				|| (score == bestScore && controller->serial < bestController->serial))
			{
				bestController = controller;
				bestScore = score;
			}
		}

		mPreferredHookedControllers[side] = bestController;
	}

	void HandOfLesser::refreshRecoveryHookedController(HOL::HandSide side)
	{
		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return;
		}

		mRecoveryHookedControllers[side] = nullptr;

		for (HookedController* controller : getHookedControllers(side))
		{
			// Full skeletal tracking indicates the dedicated hand-tracking controller pair, which is
			// not useful as the signal that native controller tracking has returned.
			if (controller->mSkeletonTrackingLevel == vr::VRSkeletalTracking_Full)
			{
				continue;
			}

			auto configIt = Config.deviceSettings.devices.find(controller->serial);
			if (configIt != Config.deviceSettings.devices.end() && configIt->second.actAsTracker)
			{
				continue;
			}

			mRecoveryHookedControllers[side] = controller;
			return;
		}
	}

	std::string HandOfLesser::getPreferredHookedControllerSerial(HOL::HandSide side) const
	{
		switch (side)
		{
			case HandSide::LeftHand:
				return Config.deviceSettings.preferredLeftControllerSerial;
			case HandSide::RightHand:
				return Config.deviceSettings.preferredRightControllerSerial;
			default:
				return "";
		}
	}

	HookedController* HandOfLesser::getRecoveryHookedController(HOL::HandSide side) const
	{
		if (side < 0 || side >= HOL::HandSide_MAX)
		{
			return nullptr;
		}

		return mRecoveryHookedControllers[side];
	}

	vr::EVRSkeletalTrackingLevel HandOfLesser::getRequestedSkeletalTrackingLevel() const
	{
		return Config.skeletal.trackingLevel;
	}

	int HandOfLesser::getHookedControllerSelectionScore(HookedController* controller) const
	{
		if (controller == nullptr)
		{
			return std::numeric_limits<int>::lowest();
		}

		int score = 0;

		// Controllers that the user turned into trackers should not also become the primary
		// possessed controller.
		auto configIt = Config.deviceSettings.devices.find(controller->serial);
		if (configIt != Config.deviceSettings.devices.end() && configIt->second.actAsTracker)
		{
			score -= 1000;
		}

		bool wantsFullTracking
			= getRequestedSkeletalTrackingLevel() == vr::VRSkeletalTracking_Full;
		bool controllerProvidesFullTracking
			= controller->mSkeletonTrackingLevel == vr::VRSkeletalTracking_Full;

		// When both controller pairs exist, prefer the device whose native skeletal tracking
		// level best matches the current request.
		if (controllerProvidesFullTracking)
		{
			score += wantsFullTracking ? 100 : -100;
		}

		// If we have ever seen a real pose from this controller, prefer it slightly over an
		// otherwise-equal candidate that has never produced one.
		if (controller->mHasHadValidOriginalPose)
		{
			score += 10;
		}

		return score;
	}

	HookedController* HandOfLesser::getHookedControllerByDeviceId(uint32_t deviceId)
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			if (controller->getDeviceId() == deviceId)
			{
				return controller;
			}
		}

		return nullptr;
	}

	HookedController* HandOfLesser::getHookedControllerBySerial(std::string serial)
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			{
				if (controller->serial == serial)
				{
					return controller;
				}
			}
		}

		return nullptr;
	}

	HookedController*
	HandOfLesser::getHookedControllerByPropertyContainer(vr::PropertyContainerHandle_t container)
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			if (controllerContainer->propertyContainer == container)
			{
				return controllerContainer.get();
			}
		}

		return nullptr;
	}

	HookedController* HandOfLesser::getHMD()
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			if (controllerContainer->mDeviceClass
				== vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
			{
				return controllerContainer.get();
			}
		}

		return nullptr;
	}

	HookedController*
	HandOfLesser::getHookedControllerByInputHandle(vr::VRInputComponentHandle_t inputHandle)
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			{
				if (controller->inputHandles.contains(inputHandle))
				{
					return controller;
				}
			}
		}

		return nullptr;
	}

	GenericControllerInterface* HandOfLesser::GetActiveController(HOL::HandSide side)
	{
		switch (HandOfLesser::Current->Config.handPose.controllerMode)
		{
			case ControllerMode::EmulateControllerMode: {
				EmulatedControllerDriver* emulated = getEmulatedController(side);
				if (emulated == nullptr)
				{
					return nullptr;
				}
				// If not connected then it is not the primary controller
				if (!emulated->isConnected())
				{
					return nullptr;
				}
				return emulated;
			}
			case ControllerMode::HookedControllerMode: {
				HookedController* hooked = getHookedController(side);
				if (hooked == nullptr)
				{
					return nullptr;
				}

				// Not active if we shouldn't possess it
				if (!shouldPossess(hooked))
				{
					return nullptr;
				}

				return hooked;
			}
			default: {
				return nullptr;
			}
		}
	}

	void HandOfLesser::requestEstimateControllerSide()
	{
		// Ideally we would just wait for the controller-has-been-assigned-a-side
		// event and immediately decide on what side each controller is.
		// However, OVR will send this event before the controller has
		// submitted a valid pose, resulting in it failing.
		// As a result of this we need to wait for all poses to be valid before
		// we run our estimation.
		this->mEstimateControllerSideWhenPositionValid = true;
	}

	void HandOfLesser::estimateControllerSide()
	{
		mControllerSideEstimationAttemptCount++;

		if (mControllerSideEstimationAttemptCount > 500)
		{
			this->mEstimateControllerSideWhenPositionValid = false;
			this->mControllerSideEstimationAttemptCount = 0;
			DriverLog("Waited 500 frames HDM and controllers to have valid positions, giving up on "
					  "deciding sides.");
			return;
		}

		// Vive wands don't know what side they are until later,
		// and we can't query what side they're assigned without
		// being a client, so instead we listen for the event
		// that says they've been assigned a side, and... guess.
		std::vector<HookedController*> unsidedControllers;

		for (auto& controller : mHookedControllers)
		{
			// Controller with no left/right role hint.
			// Only applies to Vive wand, and they use the invalid role.
			if (controller->mDeviceClass == vr::TrackedDeviceClass_Controller
				&& controller->role == vr::TrackedControllerRole_Invalid)
			{
				if (!controller->mLastOriginalPoseValid)
				{
					// DriverLog("Sided controller without valid position, skipping side
					// estimation");
					return;
				}

				unsidedControllers.push_back(controller.get());
			}
		}

		if (unsidedControllers.empty())
		{
			// uhhhhhh
			DriverLog("No unsided controllers, not peforming side estimation");
			this->mEstimateControllerSideWhenPositionValid = false;
			this->mControllerSideEstimationAttemptCount = 0;
		}

		if (unsidedControllers.size() > 2)
		{
			// uhhhhhh
			DriverLog("More than two unsided controller, so I give up. Count: %d",
					  unsidedControllers.size());
		}

		HookedController* hmd = getHMD();
		if (hmd == nullptr)
		{
			DriverLog("Could not get HMD to do handedness math, so I give up.");
			return;
		}

		if (!hmd->mLastOriginalPoseValid)
		{
			DriverLog("HMD position not valid, will not estimate controller side");
			return;
		}

		// Cross forward with 0,1,0 to get x axis on flat plane
		// This will be our plane up
		Eigen::Vector3f hmdPos = HOL::ovrVectorToEigen(hmd->lastOriginalPose.vecPosition);
		Eigen::Quaternionf hmdRot = HOL::ovrQuaternionToEigen(hmd->lastOriginalPose.qRotation);

		Eigen::Quaternionf hmdHead
			= HOL::ovrQuaternionToEigen(hmd->lastOriginalPose.qWorldFromDriverRotation);
		hmdRot = hmdRot * hmdHead;

		Eigen::Vector3f hmdForward = hmdRot * Eigen::Vector3f(0, 0, 1);
		Eigen::Vector3f hmdSide = hmdForward.cross(Eigen::Vector3f(0, 1, 0));

		if (unsidedControllers.size() == 2)
		{
			// remove y component, get angle between hmd forward and hmd->controller.
			// lower of two values is left, higher is right.

			Eigen::Vector3f controller1Pos
				= HOL::ovrVectorToEigen(unsidedControllers[0]->lastOriginalPose.vecPosition);
			Eigen::Vector3f controller2Pos
				= HOL::ovrVectorToEigen(unsidedControllers[1]->lastOriginalPose.vecPosition);

			Eigen::Vector3f controller1Vector = controller1Pos - hmdPos;
			Eigen::Vector3f controller2Vector = controller2Pos - hmdPos;

			float controller1Distance = hmdSide.dot(controller1Vector);
			float controller2Distance = hmdSide.dot(controller2Vector);

			// The more above the plane the more right the controller is.
			// Higher number is right controller, other is left.
			// OR IT SHOULD BE BUT ITS OPPOSITE AND I GIVE UP
			// I just inverted the if/else
			if (controller1Distance > controller2Distance)
			{
				unsidedControllers[0]->setSide(HandSide::LeftHand);
				unsidedControllers[1]->setSide(HandSide::RightHand);

				DriverLog("Asssigned controller %s to left", unsidedControllers[0]->serial.c_str());
				DriverLog("Asssigned controller %s to right",
						  unsidedControllers[1]->serial.c_str());
			}
			else
			{
				unsidedControllers[0]->setSide(HandSide::RightHand);
				unsidedControllers[1]->setSide(HandSide::LeftHand);

				DriverLog("Asssigned controller %s to right",
						  unsidedControllers[0]->serial.c_str());
				DriverLog("Asssigned controller %s to left", unsidedControllers[1]->serial.c_str());
			}
		}
		else if (unsidedControllers.size() == 1)
		{
			// Same general idea, but anything to right of HMD will be right and vice versa
			Eigen::Vector3f controller1Pos
				= HOL::ovrVectorToEigen(unsidedControllers[0]->lastOriginalPose.vecPosition);

			Eigen::Vector3f controller1Vector = controller1Pos - hmdPos;

			float controller1Distance = hmdSide.dot(controller1Vector);

			// right if > 0, otherwise left
			// OR IT SHOULD BE BUT ITS OPPOSITE AND I GIVE UP
			// I just inverted the if/else
			if (controller1Distance > 0)
			{
				unsidedControllers[0]->setSide(HandSide::LeftHand);
				DriverLog("Asssigned controller %s to left", unsidedControllers[0]->serial.c_str());
			}
			else
			{

				unsidedControllers[0]->setSide(HandSide::RightHand);
				DriverLog("Asssigned controller %s to right",
						  unsidedControllers[0]->serial.c_str());
			}
		}

		this->mControllerSideEstimationAttemptCount = 0;
		this->mEstimateControllerSideWhenPositionValid = false;
	}

	void HandOfLesser::runFrame()
	{
		// As of writing only emulated controllers need to do anything here
		if (HandOfLesser::Current->Config.handPose.controllerMode
			== ControllerMode::EmulateControllerMode)
		{
			// TODO: only run if using index controller
			EmulatedControllerDriver* leftController = this->getEmulatedController(HandSide::LeftHand);
			EmulatedControllerDriver* rightController
				= this->getEmulatedController(HandSide::RightHand);

			// We always create both
			if (leftController != nullptr && rightController != nullptr)
			{
				// TODO: check if activate, if we do silly things like swap between controllers

				// call our devices to run a frame
				leftController->MyRunFrame();
				rightController->MyRunFrame();

				// Now, process events that were submitted for this frame.
				vr::VREvent_t vrevent{};
				while (vr::VRServerDriverHost()->PollNextEvent(&vrevent, sizeof(vr::VREvent_t)))
				{
					leftController->MyProcessEvent(vrevent);
					rightController->MyProcessEvent(vrevent);
				}
			}
		}
		else if (HandOfLesser::Current->Config.handPose.controllerMode
				 == ControllerMode::HookedControllerMode)
		{
			// TODO: only run if using index controller
			HookedController* leftController = this->getHookedController(HandSide::LeftHand);
			HookedController* rightController = this->getHookedController(HandSide::RightHand);

			// We always create both
			if (leftController != nullptr && rightController != nullptr)
			{
				// We don't actually have anything to do for these
			}
		}

		if (this->mEstimateControllerSideWhenPositionValid)
		{
			this->estimateControllerSide();
		}

		// iterate frame counter for all controller
		for (auto& controller : this->mHookedControllers)
		{
			controller->framesSinceLastPoseUpdate++;
		}
	}

	void HandOfLesser::cleanup()
	{
		// Let's join our pose thread that's running
		// by first checking then setting is_active_ to false to break out
		// of the while loop, if it's running, then call .join() on the thread
		// if (is_active_.exchange(false))
		this->mActive = false;
		{
			my_pose_update_thread_.join();
		}

		// Our controller devices will have already deactivated. Let's now destroy them.
		this->destroyEmulatedControllers();
	}

	float HandOfLesser::getControllerToHandDistance(HookedController* controller)
	{
		if (controller == nullptr)
		{
			return (std::numeric_limits<float>::max)();
		}

		if (!Tracking.isMultimodalEnabled)
		{
			return (std::numeric_limits<float>::max)();
		}

		// Get controller side
		HandSide side = controller->getSide();
		if (side == HandSide::HandSide_MAX)
		{
			return (std::numeric_limits<float>::max)();
		}

		auto& multimodal = HOL::HandOfLesser::Current->mLastMultimodalPosePacket;

		// You would think upper-body tracking would be used to augment controller tracker
		// when it gives up tracking the controller, but no. Likewise, Controller rotation, which
		// remains available, is not used to augment the body tracking.
		// For this reason the two go out of sync when this happens, but at least they
		// mark the hand as not being tracked anymore.
		bool handValid = (side == HandSide::LeftHand) ? multimodal.leftHandTracked
													  : multimodal.rightHandTracked;
		if (!handValid)
		{
			return (std::numeric_limits<float>::max)();
		}

		// Check if controller pose is valid
		if (!controller->mLastOriginalPoseValid)
		{
			return (std::numeric_limits<float>::max)();
		}

		// Get body tracking hand pose
		HOL::PoseLocation& bodyHandPose
			= (side == HandSide::LeftHand) ? multimodal.leftHandPose : multimodal.rightHandPose;

		// Calculate and return distance
		return (controller->getWorldPosition() - bodyHandPose.position).norm();
	}

	void HandOfLesser::sendStatus()
	{
		DriverStatusPacket packet;
		packet.emulatedControllersActive = false;
		for (auto& controller : mEmulatedControllers)
		{
			if (controller != nullptr)
			{
				packet.emulatedControllersActive = true;
				break;
			}
		}

		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			if (controller->mDeviceClass != vr::TrackedDeviceClass_Controller
				|| controller->isActingAsTracker())
			{
				continue;
			}

			if (controller->mSkeletonTrackingLevel == vr::VRSkeletalTracking_Full)
			{
				packet.hasHandTrackingControllers = true;
			}
			else
			{
				packet.hasNormalControllers = true;
			}
		}

		packet.hookedControllerCount = (int)mHookedControllers.size();
		packet.emulatedTrackerCount = (int)mEmulatedTrackers.size();

		this->mTransport.send((char*)&packet, sizeof(packet));
		DriverLog("Sent driver status: emulated=%d, normal=%d, hand=%d, hooked=%d, trackers=%d",
				  packet.emulatedControllersActive,
				  packet.hasNormalControllers,
				  packet.hasHandTrackingControllers,
				  packet.hookedControllerCount,
				  packet.emulatedTrackerCount);
	}

	void HandOfLesser::sendDeviceState(HookedController* device)
	{
		DeviceStatePacket packet;
		strncpy_s(packet.serial, sizeof(packet.serial), device->serial.c_str(), _TRUNCATE);
		packet.role = device->mDeviceClass;
		packet.trackingLevel = device->mSkeletonTrackingLevel;

		this->mTransport.send((char*)&packet, sizeof(packet));
	}

	void HandOfLesser::sendAllDeviceStates()
	{
		for (auto& controller : mHookedControllers)
		{
			sendDeviceState(controller.get());
		}
	}
} // namespace HOL
