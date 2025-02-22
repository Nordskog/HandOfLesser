#include "hand_of_lesser.h"
#include "HandOfLesserCommon.h"
#include <driverlog.h>
#include "src/utils/math_utils.h"

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

	static bool lastPossessState = false;

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
				return controller->shouldPossess();
			}
		}

		return false;
	}

	bool HandOfLesser::shouldEmulateControllers()
	{
		return HandOfLesser::Current->Config.handPose.controllerMode
			   == ControllerMode::EmulateControllerMode;
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
} // namespace HOL