#pragma once

#include "generic_control_interface.h"
#include <openvr_driver.h>

namespace HOL
{
	class HookedController : public GenericControllerInterface
	{
	public:
		void UpdatePose(HOL::HandTransformPacket* packet) override;
		void UpdateInput(HOL::ControllerInputPacket* packet) override;
		void SubmitPose() override;

		private:
		vr::DriverPose_t mLastPose;

		HOL::HandTransformPacket mLastTransformPacket;
		HOL::ControllerInputPacket mLastInputPacket;
	};
}