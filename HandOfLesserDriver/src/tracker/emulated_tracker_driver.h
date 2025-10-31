//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <string>
#include <atomic>

#include "openvr_driver.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	//-----------------------------------------------------------------------------
	// Purpose: Represents a body tracker device in the system
	//-----------------------------------------------------------------------------
	class EmulatedTrackerDriver : public vr::ITrackedDeviceServerDriver
	{
	public:
		EmulatedTrackerDriver(HOL::BodyTrackerRole role);

		// ITrackedDeviceServerDriver interface
		vr::EVRInitError Activate(uint32_t unObjectId) override;
		void Deactivate() override;
		void EnterStandby() override;
		void* GetComponent(const char* pchComponentNameAndVersion) override;
		void DebugRequest(const char* pchRequest,
						  char* pchResponseBuffer,
						  uint32_t unResponseBufferSize) override;
		vr::DriverPose_t GetPose() override;

		// Custom methods
		const std::string& MyGetSerialNumber();
		void UpdatePose(const HOL::BodyTrackerPosePacket& packet);
		void SubmitPose();
		void setConnectedState(bool connected);

	private:
		HOL::BodyTrackerRole mRole;
		std::string mSerialNumber;
		std::atomic<vr::TrackedDeviceIndex_t> mDeviceIndex = vr::k_unTrackedDeviceIndexInvalid;
		std::atomic<bool> mIsActive = false;
		std::atomic<bool> mDeviceConnected = true;
		vr::DriverPose_t mLastPose;
	};

} // namespace HOL
