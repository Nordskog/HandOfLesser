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
		if (HOL::HandOfLesser::Current->shouldPossess(this->mDeviceId))
		{
			// In this state we prevent the original hooked call from running,
			// and call the original function with our pose instead.
			HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				this->mHookedHost, this->mDeviceId, this->mLastPose, sizeof(vr::DriverPose_t));		

		}
	}
	uint32_t HookedController::getDeviceId()
	{
		return this->mDeviceId;
	}

} // namespace HOL