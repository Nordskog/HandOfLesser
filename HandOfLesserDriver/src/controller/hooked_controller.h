#pragma once

#include "generic_control_interface.h"
#include <openvr_driver.h>
#include "src/input/InputCommons.h"

namespace HOL
{
	class HookedController : public GenericControllerInterface
	{
	public:
		HookedController(uint32_t id,
						 HandSide side,
						 vr::IVRServerDriverHost* host, 
						 vr::ITrackedDeviceServerDriver* driver,
						 std::string serial,
						 vr::ETrackedDeviceClass deviceClass);
		void UpdatePose(HOL::HandTransformPacket* packet) override;
		void UpdateInput(HOL::ControllerInputPacket* packet) override;
		void UpdateBoolInput(const std::string& input, bool value) override;
		void UpdateFloatInput(const std::string& input, float value) override;
		void SubmitPose() override;

		bool canPossess();
		bool shouldPossess();

		void setSide(HandSide side);
		HandSide getSide();

		void setLastOriginalPoseState(bool valid);

		vr::ETrackedDeviceClass mDeviceClass = vr::ETrackedDeviceClass::TrackedDeviceClass_Max;
		std::string serial;
		vr::IVRDriverInput* driverInput;
		std::unordered_map<vr::VRInputComponentHandle_t, ControllerInputHandle> inputHandles;
		std::unordered_map<std::string, vr::VRInputComponentHandle_t> inputHandlesByName;

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