//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <memory>

#include "openvr_driver.h"
#include <HandOfLesserCommon.h>
#include "hand_of_lesser.h"

// make sure your class is publicly inheriting vr::IServerTrackedDeviceProvider!
class MyDeviceProvider : public vr::IServerTrackedDeviceProvider
{
public:
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
	const char* const* GetInterfaceVersions() override;

	void RunFrame() override;

	bool ShouldBlockStandbyMode() override;
	void EnterStandby() override;
	void LeaveStandby() override;

	void Cleanup() override;

private:

	HOL::HandOfLesser mHandOfLesser;

	bool mActive = true; // Just so we can make the loop exit. Fix later.
};
