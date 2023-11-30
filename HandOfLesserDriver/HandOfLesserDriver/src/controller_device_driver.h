//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <array>
#include <string>

#include "openvr_driver.h"
#include <HandOfLesserCommon.h>
#include <atomic>
#include <thread>
#include "hand_simulation.h"

enum InputHandleType
{
	a_touch,
	a_click,

	trigger_value,
	trigger_touch,
	trigger_click,

	grip_value,
	grip_force,
	grip_touch,

	system_click,

	finger_index,
	finger_middle,
	finger_ring,
	finger_pinky,

	skeleton,

	haptic,

	MAX
};

//-----------------------------------------------------------------------------
// Purpose: Represents a single tracked device in the system.
// What this device actually is (controller, hmd) depends on the
// properties you set within the device (see implementation of Activate)
//-----------------------------------------------------------------------------
class ControllerDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	ControllerDeviceDriver(vr::ETrackedControllerRole role);

	vr::EVRInitError Activate(uint32_t unObjectId) override;

	void EnterStandby() override;

	void* GetComponent(const char* pchComponentNameAndVersion) override;

	void DebugRequest(
		const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize
	) override;

	vr::DriverPose_t GetPose() override;

	void UpdatePose(HOL::HandTransformPacket* packet);
	void UpdateInput(HOL::ControllerInputPacket* packet);

	void SubmitPose();

	void Deactivate() override;

	// ----- Functions we declare ourselves below -----

	const std::string& MyGetSerialNumber();

	void MyRunFrame();
	void MyProcessEvent(const vr::VREvent_t& vrevent);

	void MyPoseUpdateThread();

private:
	void UpdateSkeleton();

	std::unique_ptr<MyHandSimulation> my_hand_simulation_;
	std::atomic<int> frame_ = 0;
	std::atomic<float> last_curl_ = 0.f;
	std::atomic<float> last_splay_ = 0.f;

	std::atomic<vr::TrackedDeviceIndex_t> my_controller_index_;

	vr::ETrackedControllerRole my_controller_role_;

	std::string my_controller_model_number_;
	std::string my_controller_serial_number_;

	std::array<vr::VRInputComponentHandle_t, InputHandleType::MAX> mInputHandles;

	std::atomic<bool> is_active_;

	vr::DriverPose_t mLastPose;

	HOL::HandTransformPacket mLastTransformPacket;
	HOL::ControllerInputPacket mLastInputPacket;
};
