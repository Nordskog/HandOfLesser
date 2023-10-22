//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <memory>

#include "controller_device_driver.h"
#include "openvr_driver.h"
#include <HandOfLesserCommon.h>

// make sure your class is publicly inheriting vr::IServerTrackedDeviceProvider!
class MyDeviceProvider : public vr::IServerTrackedDeviceProvider
{
public:
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
	const char* const* GetInterfaceVersions() override;

	void RunFrame() override;
	void ReceiveDataThread();

	bool ShouldBlockStandbyMode() override;
	void EnterStandby() override;
	void LeaveStandby() override;

	void Cleanup() override;

private:
	std::unique_ptr<ControllerDeviceDriver> my_left_controller_device_;
	std::unique_ptr<ControllerDeviceDriver> my_right_controller_device_;
	std::thread my_pose_update_thread_;
	HOL::NativeTransport mTransport;
	bool mActive = true; // Just so we can make the loop exit. Fix later.
};
