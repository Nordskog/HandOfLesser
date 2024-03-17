#pragma once

#include "src/core/hand_of_lesser.h"
#include "hooked_controller.h"
#include "controller_common.h"
#include "src/hooking/hooks.h"

namespace HOL
{
	HookedController::HookedController(uint32_t id,
									   HandSide side,
									   vr::IVRServerDriverHost* host,
									   vr::ITrackedDeviceServerDriver* driver)
	{
		this->mSide = side;
		this->mDeviceId = id;
		this->mHookedHost = host;
		this->mHookedDriver = driver;
	}

	void HookedController::UpdatePose(HOL::HandTransformPacket* packet)
	{
		// packet data resides in receive buffer and will be replaced on next receive,
		// so make a copy now.
		this->mLastTransformPacket = *packet;

		// Store the pose somewhere
		this->mLastPose = ControllerCommon::generatePose(&this->mLastTransformPacket, true);
	}
	void HookedController::UpdateInput(HOL::ControllerInputPacket* packet)
	{

	}
	void HookedController::SubmitPose()
	{
		if (HOL::HandOfLesser::Current->shouldPossess(this) && this->poseValid())
		{
			// In this state we prevent the original hooked call from running,
			// and call the original function with our pose instead.
			HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				this->mHookedHost, this->mDeviceId, this->mLastPose, sizeof(vr::DriverPose_t));		

		}
	}

	// Can, not should.
	bool HookedController::canPossess()
	{
		// Active: indicating if the hand tracker is actively tracking.
		// On Quest2 this is false as soon as the hand is lost.
		// On Quest3 you might expect it to remain active at all times
		// because of arm tracking and stuff. See HookedController::poseValid()

		// As of writing broken in VD because VDXR broken. Works with Airlink.
		// Airlink should potentially check controller state instead.
		// Other controllers ( Lighthouse anything ) will need some system to tell 
		// when controllers are placed aside and the user wants to use handtracking instead.
		return this->mLastTransformPacket.active;
	}

	bool HookedController::poseValid()
	{
		// In addition to active, we also have valid and tracked bits.
		// When active, either of these may be false.
		// Quest3 may have a valid but untracked position for the palm.
		// If active is false, valid and tracked will always be false too.
		return this->mLastTransformPacket.valid;
	}



	uint32_t HookedController::getDeviceId()
	{
		return this->mDeviceId;
	}

} // namespace HOL