//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "emulated_tracker_driver.h"

#include "driverlog.h"
#include "../controller/controller_common.h"
#include <src/core/hand_of_lesser.h>

namespace HOL
{
	EmulatedTrackerDriver::EmulatedTrackerDriver(HOL::BodyTrackerRole role)
	{
		mIsActive = false;
		mRole = role;
		mSerialNumber = bodyTrackerRoleToSerial(role);

		DriverLog("HandOfLesser tracker created: %s (serial: %s)",
				  bodyTrackerRoleToString(role),
				  mSerialNumber.c_str());
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called by vrserver after TrackedDeviceAdded
	//-----------------------------------------------------------------------------
	vr::EVRInitError EmulatedTrackerDriver::Activate(uint32_t unObjectId)
	{
		mDeviceIndex = unObjectId;

		auto props = vr::VRProperties();
		vr::PropertyContainerHandle_t container
			= props->TrackedDeviceToPropertyContainer(mDeviceIndex);

		// Set basic tracker properties
		props->SetStringProperty(container, vr::Prop_ModelNumber_String, "HandOfLesser Tracker");
		props->SetStringProperty(
			container, vr::Prop_SerialNumber_String, mSerialNumber.c_str());

		// Reference HTC Vive Tracker resources
		props->SetStringProperty(
			container, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
		props->SetStringProperty(container, vr::Prop_ResourceRoot_String, "htc");

		// Set tracker role for proper icon/functionality
		const char* trackerRole = bodyTrackerRoleToTrackerRoleString(mRole);
		props->SetStringProperty(container, vr::Prop_ControllerType_String, trackerRole);

		// Reference HTC's input profile (minimal - just pose)
		props->SetStringProperty(
			container, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json");

		// Set device class
		props->SetInt32Property(
			container, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);

		// Initialize pose
		mLastPose = ControllerCommon::generateDisconnectedPose();

		mIsActive = true;

		DriverLog("HandOfLesser tracker activated: %s", bodyTrackerRoleToString(mRole));

		return vr::VRInitError_None;
	}

	void EmulatedTrackerDriver::Deactivate()
	{
		// Clear device ID
		mDeviceIndex = vr::k_unTrackedDeviceIndexInvalid;
		mIsActive = false;
	}

	void EmulatedTrackerDriver::EnterStandby()
	{
		// Nothing to do
	}

	void* EmulatedTrackerDriver::GetComponent(const char* pchComponentNameAndVersion)
	{
		// No components needed for basic tracker
		return nullptr;
	}

	void EmulatedTrackerDriver::DebugRequest(const char* pchRequest,
											  char* pchResponseBuffer,
											  uint32_t unResponseBufferSize)
	{
		// No debug requests supported
		if (unResponseBufferSize >= 1)
		{
			pchResponseBuffer[0] = 0;
		}
	}

	vr::DriverPose_t EmulatedTrackerDriver::GetPose()
	{
		return mLastPose;
	}

	const std::string& EmulatedTrackerDriver::MyGetSerialNumber()
	{
		return mSerialNumber;
	}

	void EmulatedTrackerDriver::UpdatePose(const HOL::BodyTrackerPosePacket& packet)
	{
		// Build DriverPose_t from packet
		mLastPose.qWorldFromDriverRotation.w = 1.0;
		mLastPose.qWorldFromDriverRotation.x = 0.0;
		mLastPose.qWorldFromDriverRotation.y = 0.0;
		mLastPose.qWorldFromDriverRotation.z = 0.0;

		mLastPose.qDriverFromHeadRotation.w = 1.0;
		mLastPose.qDriverFromHeadRotation.x = 0.0;
		mLastPose.qDriverFromHeadRotation.y = 0.0;
		mLastPose.qDriverFromHeadRotation.z = 0.0;

		// Set position
		mLastPose.vecPosition[0] = packet.location.position.x();
		mLastPose.vecPosition[1] = packet.location.position.y();
		mLastPose.vecPosition[2] = packet.location.position.z();

		// Set orientation
		mLastPose.qRotation.w = packet.location.orientation.w();
		mLastPose.qRotation.x = packet.location.orientation.x();
		mLastPose.qRotation.y = packet.location.orientation.y();
		mLastPose.qRotation.z = packet.location.orientation.z();

		// Set velocities
		mLastPose.vecVelocity[0] = packet.velocity.linearVelocity.x();
		mLastPose.vecVelocity[1] = packet.velocity.linearVelocity.y();
		mLastPose.vecVelocity[2] = packet.velocity.linearVelocity.z();

		mLastPose.vecAngularVelocity[0] = packet.velocity.angularVelocity.x();
		mLastPose.vecAngularVelocity[1] = packet.velocity.angularVelocity.y();
		mLastPose.vecAngularVelocity[2] = packet.velocity.angularVelocity.z();

		// Set tracking state
		mLastPose.poseIsValid = packet.valid;
		mLastPose.deviceIsConnected = mDeviceConnected;
		mLastPose.result = packet.tracked ? vr::TrackingResult_Running_OK
										  : vr::TrackingResult_Running_OutOfRange;

		mLastPose.poseTimeOffset = 0.0;
	}

	void EmulatedTrackerDriver::SubmitPose()
	{
		if (mIsActive && mDeviceConnected)
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(
				mDeviceIndex, GetPose(), sizeof(vr::DriverPose_t));
		}
	}

	void EmulatedTrackerDriver::setConnectedState(bool connected)
	{
		if (mDeviceConnected == connected)
			return;

		mDeviceConnected = connected;

		if (!connected)
		{
			// Submit disconnected pose
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(
				mDeviceIndex, ControllerCommon::generateDisconnectedPose(), sizeof(vr::DriverPose_t));
		}
	}

} // namespace HOL
