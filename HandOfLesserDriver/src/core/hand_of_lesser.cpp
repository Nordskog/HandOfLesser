#include "hand_of_lesser.h"
#include "HandOfLesserCommon.h"
#include <driverlog.h>
#include "src/utils/math_utils.h"

namespace HOL
{

	HandOfLesser* HandOfLesser::Current = nullptr;
	HOL::settings::HandOfLesserSettings HandOfLesser::Config;
	HOL::state::TrackingState HandOfLesser::Tracking;
	HOL::state::RuntimeState HandOfLesser::Runtime;

	void HandOfLesser::init()
	{
		this->mActive = true;
		HandOfLesser::Current = this;
		this->mTransport.init(9006); // Hardcoded for now, needs to be negotaited somehow
		my_pose_update_thread_ = std::thread(&HandOfLesser::ReceiveDataThread, this);
	}

	void HandOfLesser::ReceiveDataThread()
	{
		DriverLog("ReceiveDataThread() start, active: %s", this->mActive ? "true" : "false");
		while (this->mActive)
		{
			HOL::NativePacket* rawPacket = this->mTransport.receive();
			if (rawPacket == nullptr)
			{
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

					if (Config.handPose.controllerMode != ControllerMode::HookedControllerMode
						&& Config.handPose.controllerMode != ControllerMode::OffsetControllerMode)
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

				case HOL::NativePacketType::ControllerInput: {
					HOL::ControllerInputPacket* packet = (HOL::ControllerInputPacket*)rawPacket;

					if (packet->valid)
					{
						GenericControllerInterface* controller
							= this->GetActiveController(packet->side);
						if (controller != nullptr)
						{
							controller->UpdateInput(packet);
						}
					}

					break;
				}

				case HOL::NativePacketType::Settings: {
					HOL::SettingsPacket* packet = (HOL::SettingsPacket*)rawPacket;

					HOL::settings::HandOfLesserSettings oldSettings = Config;
					HandOfLesser::Config = packet->config;

					// Handle any configuration changes
					handleConfigurationChange(oldSettings);

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
					auto controller = this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdateSkeletal(packet);
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

				default: {
					// Invalid packet type!
				}
			}
		}
	}

	void HandOfLesser::handleConfigurationChange(HOL::settings::HandOfLesserSettings& oldConfig)
	{
		// Add or remove ( or enable/disable ) emulated controllers on controller mode change
		if (Config.handPose.controllerMode != oldConfig.handPose.controllerMode)
		{
			if (Config.handPose.controllerMode == ControllerMode::EmulateControllerMode)
			{
				addEmulatedControllers();
			}
			else if (oldConfig.handPose.controllerMode == ControllerMode::EmulateControllerMode)
			{
				removeEmulatedControllers();
			}
		}

		// Handle body tracker changes
		bool trackersEnabledChanged
			= Config.bodyTrackers.enableBodyTrackers != oldConfig.bodyTrackers.enableBodyTrackers;
		bool anyTrackerSettingChanged
			= Config.bodyTrackers.enableHips != oldConfig.bodyTrackers.enableHips
			  || Config.bodyTrackers.enableChest != oldConfig.bodyTrackers.enableChest
			  || Config.bodyTrackers.enableLeftShoulder != oldConfig.bodyTrackers.enableLeftShoulder
			  || Config.bodyTrackers.enableLeftUpperArm
					 != oldConfig.bodyTrackers.enableLeftUpperArm
			  || Config.bodyTrackers.enableLeftLowerArm
					 != oldConfig.bodyTrackers.enableLeftLowerArm
			  || Config.bodyTrackers.enableRightShoulder
					 != oldConfig.bodyTrackers.enableRightShoulder
			  || Config.bodyTrackers.enableRightUpperArm
					 != oldConfig.bodyTrackers.enableRightUpperArm
			  || Config.bodyTrackers.enableRightLowerArm
					 != oldConfig.bodyTrackers.enableRightLowerArm;

		if (trackersEnabledChanged || anyTrackerSettingChanged)
		{
			if (Config.bodyTrackers.enableBodyTrackers)
			{
				addEmulatedTrackers();
			}
			else
			{
				removeEmulatedTrackers();
			}
		}

		// Update logic wouldn't normally run unless in certain modes, so force upon config change.
		updateControllerConnectionStates(true);
		updateTrackerConnectionStates();
	}

	void HandOfLesser::addEmulatedControllers()
	{
		if (this->mEmulatedControllers[HOL::HandSide::LeftHand])
		{
			// We only add controllers once, and then enable/disable them.
			this->mEmulatedControllers[HOL::HandSide::LeftHand]->setConnectedState(true);
			this->mEmulatedControllers[HOL::HandSide::RightHand]->setConnectedState(true);
		}
		else
		{
			// Let's add our controllers to the system.
			// First, we need to actually instantiate our controller devices.
			// We made the constructor take in a controller role, so let's pass their respective
			// roles in.
			this->mEmulatedControllers[HOL::HandSide::LeftHand]
				= std::make_unique<EmulatedControllerDriver>(vr::TrackedControllerRole_LeftHand);
			this->mEmulatedControllers[HOL::HandSide::RightHand]
				= std::make_unique<EmulatedControllerDriver>(vr::TrackedControllerRole_RightHand);

			// Now we need to tell vrserver about our controllers.
			// The first argument is the serial number of the device, which must be unique across
			// all devices. We get it from our driver settings when we instantiate, And can pass it
			// out of the function with MyGetSerialNumber(). Let's add the left hand controller
			// first (there isn't a specific order). make sure we actually managed to create the
			// device. TrackedDeviceAdded returning true means we have had our device added to
			// SteamVR. TrackedDeviceAdded returning true means we have had our device added to
			// SteamVR.

			// Make sure we actually managed to create the device.
			// TrackedDeviceAdded returning true means we have had our device added to SteamVR.

			for (int i = 0; i < HOL::HandSide_MAX; i++)
			{
				EmulatedControllerDriver* controller = this->mEmulatedControllers[i].get();

				if (!vr::VRServerDriverHost()->TrackedDeviceAdded(
						controller->MyGetSerialNumber().c_str(),
						vr::TrackedDeviceClass_Controller,
						controller))
				{
					DriverLog("Failed to create %s controller device!",
							  (i == 0 ? "left" : "right"));
				}
			}
		}

		updateControllerConnectionStates();
	}

	void HandOfLesser::removeEmulatedControllers()
	{
		// If no pointer, then we never even added the controllers.
		if (this->mEmulatedControllers[HOL::HandSide::LeftHand])
		{
			// We only add controllers once, and then enable/disable them.
			this->mEmulatedControllers[HOL::HandSide::LeftHand]->setConnectedState(false);
			this->mEmulatedControllers[HOL::HandSide::RightHand]->setConnectedState(false);
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
					case BodyTrackerRole::LeftShoulder:
						shouldBeEnabled = Config.bodyTrackers.enableLeftShoulder;
						break;
					case BodyTrackerRole::LeftUpperArm:
						shouldBeEnabled = Config.bodyTrackers.enableLeftUpperArm;
						break;
					case BodyTrackerRole::LeftLowerArm:
						shouldBeEnabled = Config.bodyTrackers.enableLeftLowerArm;
						break;
					case BodyTrackerRole::RightShoulder:
						shouldBeEnabled = Config.bodyTrackers.enableRightShoulder;
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
					case BodyTrackerRole::LeftShoulder:
						shouldBeConnected = Config.bodyTrackers.enableLeftShoulder;
						break;
					case BodyTrackerRole::LeftUpperArm:
						shouldBeConnected = Config.bodyTrackers.enableLeftUpperArm;
						break;
					case BodyTrackerRole::LeftLowerArm:
						shouldBeConnected = Config.bodyTrackers.enableLeftLowerArm;
						break;
					case BodyTrackerRole::RightShoulder:
						shouldBeConnected = Config.bodyTrackers.enableRightShoulder;
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
		for (int i = 0; i < HOL::HandSide_MAX; ++i)
		{
			auto side = static_cast<HOL::HandSide>(i);

			bool emulationMode
				= Config.handPose.controllerMode == ControllerMode::EmulateControllerMode;

			if (forceUpdate || emulationMode)
			{
				bool handTrackingPrimary = emulationMode && isHandTrackingPrimary(side);

				// TODO: Add bool to determine if we should suppress existing controllers
				if (auto hooked = getHookedController(side))
				{
					// We don't want to spam disconnect signals, but we want to ensure that the
					// hooked controllers are reset to their original state after we've
					// possess or suppressed them. If the controllers are connected and active
					// their next pose submission will connect them again, so we can safely send
					// a disconnect here, cover the case when the controller is should be inactive.
					if (forceUpdate)
					{
						hooked->sendDisconnectState();
					}

					// This will set suppressed state and a disconnect event depending on the existing
					// suppression state, meaning it may not trigger if we've been submitting poses
					// on its behalf in possession or offset modes.
					// With multimodal enabled all
					hooked->setSuppressed(handTrackingPrimary );
				}
				

				// Just sets an internal bool to enabled so it accepts data,
				// and we can tell whether or not it was active when we disable it.
				if (auto emulated = getEmulatedController(side))
				{
					emulated->setConnectedState(handTrackingPrimary);
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

		return this->mHookedControllers.back().get();
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
		// How we decide on whether or not to take control will depend on
		// the user's setup. With Quest1/2/pro we can assume the controllers
		// will not be active any time we have valid handtracking data
		// This holds true even if VD is emulating controllers.
		//
		// The Quest3 can do controller and hand-tracking simultaneously,
		// and its upper-body-tracking stuff means the data will probably be valid.
		//
		// Other lighthouse devices will pr obably always remain active, so I guess we just
		// wait for the controllers to be completely still while the user is holding up their
		// hands for a few seconds. Also a problem for later.
		if (HandOfLesser::Current->Config.handPose.controllerMode
			== ControllerMode::HookedControllerMode)
		{
			// Only possess the controllers we are hooking
			if (controller != nullptr)
			{
				return shouldUseHandTracking(controller);
			}
		}

		return false;
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
			bool shouldPossess = !controller->isHeld();
			return shouldPossess;
		}

		bool canPoss = controller->canPossess();
		if (!controller->mLastOriginalPoseValid && canPoss)
		{
			controller->mValidWhileOriginalInvalid = true;
		}

		if (controller->mLastOriginalPoseValid)
		{
			controller->mValidWhileOriginalInvalid = false;
		}

		bool originalSubmitStale = controller->framesSinceLastPoseUpdate > 30;

		bool shouldPossess
			= controller->mLastTransformPacket.tracked
			  || (canPoss && (!controller->mLastOriginalPoseValid || originalSubmitStale))
			  || controller->mValidWhileOriginalInvalid;

		return shouldPossess;
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
		return this->mEmulatedControllers[(int)side].get();
	}

	bool HandOfLesser::isEmulatedController(vr::ITrackedDeviceServerDriver* driver)
	{
		for (auto& controllerContainer : mEmulatedControllers)
		{
			EmulatedControllerDriver* controller = controllerContainer.get();
			if (controller == driver)
			{
				return true;
			}
		}

		return false;
	}

	// Only works if a side has been assigned
	// usually is, just not right after being activated.
	HookedController* HandOfLesser::getHookedController(HOL::HandSide side)
	{
		for (auto& controllerContainer : mHookedControllers)
		{
			HookedController* controller = controllerContainer.get();
			if (controller->getSide() == side)
			{
				return controller;
			}
		}

		return nullptr;
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
				return getEmulatedController(side);
			}
			case ControllerMode::OffsetControllerMode:
			case ControllerMode::HookedControllerMode: {
				return getHookedController(side);
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
			EmulatedControllerDriver* leftController
				= this->mEmulatedControllers[HandSide::LeftHand].get();
			EmulatedControllerDriver* rightController
				= this->mEmulatedControllers[HandSide::RightHand].get();

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
		this->mEmulatedControllers[HandSide::LeftHand].reset();
		this->mEmulatedControllers[HandSide::RightHand].reset();
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
} // namespace HOL
