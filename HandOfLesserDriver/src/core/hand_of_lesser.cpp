#include "hand_of_lesser.h"
#include <driverlog.h>

namespace HOL
{
	void HandOfLesser::init()
	{
		this->mControllerMode = ControllerMode::HookedControllerMode;
		this->mTransport.init(9006); // Hardcoded for now, needs to be negotaited somehow
		my_pose_update_thread_ = std::thread(&HandOfLesser::ReceiveDataThread, this);
	}

	void HandOfLesser::ReceiveDataThread()
	{
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

					if (packet->valid)
					{
						GenericControllerInterface* controller = this->GetActiveController(packet->side);
						if (controller != nullptr)
						{
							controller->UpdatePose(packet);
							controller->SubmitPose();
						}
					}

					break;
				}

				case HOL::NativePacketType::ControllerInput: {
					HOL::ControllerInputPacket* packet = (HOL::ControllerInputPacket*)rawPacket;

					if (packet->valid)
					{
						GenericControllerInterface* controller = this->GetActiveController(packet->side);
						if (controller != nullptr)
						{
							controller->UpdateInput(packet);
						}
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
				DriverLog("Failed to create %s controller device!", (i == 0 ? "left" : "right") );
			}
		}
	}

	EmulatedControllerDriver* HandOfLesser::getEmulatedController(HOL::HandSide side)
	{
		return this->mEmulatedControllers[(int)side].get();
	}

	HookedController* HandOfLesser::getHookedController(HOL::HandSide side)
	{
		return this->mHookedControllers[(int)side].get();
	}

	GenericControllerInterface* HandOfLesser::GetActiveController(HOL::HandSide side)
	{
		switch (this->mControllerMode)
		{
			case ControllerMode::EmulateControllerMode:
			{
				return getEmulatedController(side);
			}
			case ControllerMode::HookedControllerMode: 
			{
				return getEmulatedController(side);
			}
			default:
			{
				return nullptr;
			}
		}
	}

	void HandOfLesser::runFrame()
	{
		// As of writing only emulated controllers need to do anything here
		if (this->mControllerMode == ControllerMode::EmulateControllerMode)
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
}