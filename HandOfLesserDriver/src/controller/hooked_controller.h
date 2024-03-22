#pragma once

#include "generic_control_interface.h"
#include <openvr_driver.h>

namespace HOL
{
	class HookedController : public GenericControllerInterface
	{
	public:
		HookedController(uint32_t id,
						 HandSide side,
						 vr::IVRServerDriverHost* host, 
						 vr::ITrackedDeviceServerDriver* driver);
		void UpdatePose(HOL::HandTransformPacket* packet) override;
		void UpdateInput(HOL::ControllerInputPacket* packet) override;
		void SubmitPose() override;

		bool canPossess();
		bool shouldPossess();

		void setLastOriginalPoseState(bool valid);

		uint32_t getDeviceId();

	private:
		vr::DriverPose_t mLastPose;

		HOL::HandTransformPacket mLastTransformPacket;
		HOL::ControllerInputPacket mLastInputPacket;

		bool mLastOriginalPoseValid;
		bool mValidWhileOriginalInvalid;

		HandSide mSide;
		vr::IVRServerDriverHost* mHookedHost;
		vr::ITrackedDeviceServerDriver* mHookedDriver;
		uint32_t mDeviceId;
	};
}