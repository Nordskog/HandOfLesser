//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <string>
#include <atomic>
#include <optional>

#include "openvr_driver.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	//-----------------------------------------------------------------------------
	// Purpose: Represents a body tracker or shadow tracker device in the system
	//-----------------------------------------------------------------------------
	class EmulatedTrackerDriver : public vr::ITrackedDeviceServerDriver
	{
	public:
		// For body trackers (with predefined role)
		EmulatedTrackerDriver(HOL::BodyTrackerRole role);

		// For shadow trackers (no body role, mirrors another device)
		EmulatedTrackerDriver(const std::string& sourceSerial);

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
		void UpdatePose(const vr::DriverPose_t& pose);  // Generic pose update
		void SubmitPose();
		void setConnectedState(bool connected);
		
		bool isShadowTracker() const { return !mRole.has_value(); }

	private:
		std::optional<HOL::BodyTrackerRole> mRole;  // Empty for shadow trackers
		std::string mSourceSerial;                   // For shadow trackers
		std::string mSerialNumber;
		std::atomic<vr::TrackedDeviceIndex_t> mDeviceIndex = vr::k_unTrackedDeviceIndexInvalid;
		std::atomic<bool> mIsActive = false;
		std::atomic<bool> mDeviceConnected = true;
		vr::DriverPose_t mLastPose;
	};

} // namespace HOL
