#include "hand_of_lesser.h"
#include "HandOfLesserCommon.h"
#include <driverlog.h>

namespace HOL
{

	HandOfLesser* HandOfLesser::Current = nullptr;
	HOL::settings::HandOfLesserSettings HandOfLesser::Config;

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

					GenericControllerInterface* controller
						= this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdatePose(packet);
						controller->SubmitPose();
					}

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

					HandOfLesser::Config = packet->config;

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

				case HOL::NativePacketType::BoolInput:  {
					HOL::BoolInputPacket* packet = (HOL::BoolInputPacket*)rawPacket;
					auto controller = this->GetActiveController(packet->side);
					if (controller != nullptr)
					{
						controller->UpdateBoolInput(packet->inputName, packet->value);
					}

					break;
				}

				default: {
					// Invalid packet type!
				}
			}
		}
	}

	void HandOfLesser::addControllers()
	{
		// Let's add our controllers to the system.
		// First, we need to actually instantiate our controller devices.
		// We made the constructor take in a controller role, so let's pass their respective roles
		// in.
		this->mEmulatedControllers[HOL::HandSide::LeftHand]
			= std::make_unique<EmulatedControllerDriver>(vr::TrackedControllerRole_LeftHand);
		this->mEmulatedControllers[HOL::HandSide::RightHand]
			= std::make_unique<EmulatedControllerDriver>(vr::TrackedControllerRole_RightHand);

		// Now we need to tell vrserver about our controllers.
		// The first argument is the serial number of the device, which must be unique across all
		// devices. We get it from our driver settings when we instantiate, And can pass it out of
		// the function with MyGetSerialNumber(). Let's add the left hand controller first (there
		// isn't a specific order). make sure we actually managed to create the device.
		// TrackedDeviceAdded returning true means we have had our device added to SteamVR.
		// TrackedDeviceAdded returning true means we have had our device added to SteamVR.

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
				DriverLog("Failed to create %s controller device!", (i == 0 ? "left" : "right"));
			}
		}
	}

	HookedController* HandOfLesser::addHookedController(uint32_t id,
														vr::IVRServerDriverHost* host,
														vr::ITrackedDeviceServerDriver* driver,
														std::string serial,
														vr::ETrackedDeviceClass deviceClass)
	{
		// Check for existing
		for (auto& controllerContainer : mHookedControllers)
		{
			if (controllerContainer->serial == serial)
			{
				// Just clear the existing pointer and reuse it
				// Sooner or later we are going to have to care about
				// thread safety, but not today!
				controllerContainer.reset();
				controllerContainer = std::make_unique<HookedController>(
					id, HandSide::HandSide_MAX, host, driver, serial, deviceClass);

				return controllerContainer.get();
			}
		}

		// otherwise just add
		this->mHookedControllers.push_back(std::make_unique<HookedController>(
			id, HandSide::HandSide_MAX, host, driver, serial, deviceClass));

		return this->mHookedControllers.back().get();
	}

	static bool lastPossessState = false;

	bool HandOfLesser::shouldPossess(uint32_t deviceId)
	{
		if (HandOfLesser::Current->Config.handPose.mControllerMode
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
		if (HandOfLesser::Current->Config.handPose.mControllerMode
			== ControllerMode::HookedControllerMode)
		{
			// Only possess the controllers we are hooking
			if (controller != nullptr)
			{
				return controller->shouldPossess();
			}
		}

		return false;
	}

	bool HandOfLesser::shouldEmulateControllers()
	{
		return HandOfLesser::Current->Config.handPose.mControllerMode
			   == ControllerMode::EmulateControllerMode;
	}

	EmulatedControllerDriver* HandOfLesser::getEmulatedController(HOL::HandSide side)
	{
		return this->mEmulatedControllers[(int)side].get();
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
		switch (HandOfLesser::Current->Config.handPose.mControllerMode)
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

	void HandOfLesser::runFrame()
	{
		// As of writing only emulated controllers need to do anything here
		if (HandOfLesser::Current->Config.handPose.mControllerMode
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
		else if (HandOfLesser::Current->Config.handPose.mControllerMode
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
} // namespace HOL