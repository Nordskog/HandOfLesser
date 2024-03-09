#pragma once

#include "hooked_controller.h"
#include "controller_common.h"

namespace HOL
{
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