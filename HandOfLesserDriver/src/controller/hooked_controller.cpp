#pragma once

#include "hooked_controller.h"
#include "controller_common.h"

namespace HOL
{
	HookedController::HookedController(uint32_t id,
									   HandSide side,
									   vr::ITrackedDeviceServerDriver* driver)
	{
		this->mSide = side;
		this->mDeviceId = id;
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


	}
} // namespace HOL