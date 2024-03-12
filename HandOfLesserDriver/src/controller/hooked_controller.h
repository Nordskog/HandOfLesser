#pragma once

#include "generic_control_interface.h"
#include <openvr_driver.h>

namespace HOL
{
	class HookedController : public GenericControllerInterface
	{
	public:
		HookedController(uint32_t id, HandSide side, vr::ITrackedDeviceServerDriver* driver);
		void UpdatePose(HOL::HandTransformPacket* packet) override;
		void UpdateInput(HOL::ControllerInputPacket* packet) override;
		void SubmitPose() override;

	private:
		vr::DriverPose_t mLastPose;

		HOL::HandTransformPacket mLastTransformPacket;
		HOL::ControllerInputPacket mLastInputPacket;

		HandSide mSide;
		vr::ITrackedDeviceServerDriver* mHookedDriver;
		uint32_t mDeviceId;
	};
}