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

		// Do not update pose if invalid, because we want to continue submitting 
		// the last valid one. Is this necessary? is there some kind of timeout?
		if (this->mLastTransformPacket.valid)
		{
			this->mLastPose = ControllerCommon::generatePose(&this->mLastTransformPacket, true);
		}
	}
	void HookedController::UpdateInput(HOL::ControllerInputPacket* packet)
	{

	}
	void HookedController::SubmitPose()
	{
		if (HOL::HandOfLesser::Current->shouldPossess(this))
		{
			// In this state we prevent the original hooked call from running,
			// and call the original function with our pose instead.

			// If we are submitting a stale pose to lock it in place, we must jitter it
			// because vrchat is stupid and ignores all the status information steamvr provides.
			const auto& pose = (this->mLastTransformPacket.valid)
							? this->mLastPose
							: HOL::ControllerCommon::addJitter(this->mLastPose);
			
			HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				this->mHookedHost, this->mDeviceId, pose, sizeof(vr::DriverPose_t));	
	

		}
	}

	// Can, not should.
	bool HookedController::canPossess()
	{
		// Whether or not the pose is valid pretty much.
		return this->mLastTransformPacket.valid;
	}

	// Assuming other external conditions also say it should.
	bool HookedController::shouldPossess()
	{
		// Ideally we should only posses if the controllers are not 
		// being held by the user, but this is difficult to detect.
		// Quest2 using Airlink will diconnect, probably Quest3 too.
		// With VD? Impossible at the moment.
		// Lighthouse? I guess we check if the 
		// Until I have actual other hardware to test with, possesss
		// anytime handtracking is valid. If using Airlink, we continue
		// possessing until real controllers come back online.

		bool canPoss = canPossess();
		if (!mLastOriginalPoseValid && canPoss)
		{
			// We want to continue possessing while the real controllers
			// are inactive, so it doesn't immediatley jump to their pose instead.
			// This is of course meaningless for VD, since the controllers are always active.
			this->mValidWhileOriginalInvalid = true;
		}

		if (mLastOriginalPoseValid)
		{
			// Only keep posessing while handtracking invalid
			// until original pose becomes valid. 
			mValidWhileOriginalInvalid = false;
		}

		return canPoss || this->mValidWhileOriginalInvalid;
	}

	void HookedController::setLastOriginalPoseState(bool valid)
	{
		this->mLastOriginalPoseValid = valid;
	}

	uint32_t HookedController::getDeviceId()
	{
		return this->mDeviceId;
	}

} // namespace HOL