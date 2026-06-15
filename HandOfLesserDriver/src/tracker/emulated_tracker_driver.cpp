//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "emulated_tracker_driver.h"

#include "driverlog.h"
#include "../controller/controller_common.h"
#include <src/core/hand_of_lesser.h>

namespace HOL
{
	std::string EmulatedTrackerDriver::getTrackingSystemName() const
	{
		auto props = vr::VRProperties();

		if (isShadowTracker())
		{
			HookedController* sourceController
				= HandOfLesser::Current->getHookedControllerBySerial(mSourceSerial);
			if (sourceController != nullptr)
			{
				return props->GetStringProperty(sourceController->propertyContainer,
												vr::Prop_TrackingSystemName_String);
			}
		}
		else
		{
			HookedController* hmd = HandOfLesser::Current->getHMD();
			if (hmd != nullptr)
			{
				return props->GetStringProperty(
					hmd->propertyContainer, vr::Prop_TrackingSystemName_String);
			}
		}

		return {};
	}

	EmulatedTrackerDriver::EmulatedTrackerDriver(HOL::BodyTrackerRole role)
	{
		mIsActive = false;
		mRole = role;
		mSerialNumber = bodyTrackerRoleToSerial(role);

		DriverLog("HandOfLesser body tracker created: %s (serial: %s)",
				  bodyTrackerRoleToString(role),
				  mSerialNumber.c_str());
	}

	EmulatedTrackerDriver::EmulatedTrackerDriver(const std::string& sourceSerial)
	{
		mIsActive = false;
		mRole = std::nullopt;  // No body role for shadow trackers
		mSourceSerial = sourceSerial;
		mSerialNumber = "HOLSHADOW_" + sourceSerial;

		DriverLog("HandOfLesser shadow tracker created for: %s (serial: %s)",
				  sourceSerial.c_str(),
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

		// Keep the original model naming so SteamVR presentation remains unchanged.
		if (isShadowTracker())
		{
			props->SetStringProperty(container, vr::Prop_ModelNumber_String,
									 "HandOfLesser Shadow Tracker");
		}
		else
		{
			props->SetStringProperty(container, vr::Prop_ModelNumber_String,
									 "HandOfLesser Tracker");
		}
		props->SetStringProperty(
			container, vr::Prop_SerialNumber_String, mSerialNumber.c_str());
		props->SetStringProperty(container,
								 vr::Prop_RegisteredDeviceType_String,
								 ("htc/vive_tracker" + mSerialNumber).c_str());

		// Reference HTC Vive Tracker resources
		props->SetStringProperty(
			container, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
		props->SetStringProperty(container, vr::Prop_ResourceRoot_String, "htc");

		props->SetStringProperty(container, vr::Prop_ControllerType_String, "vive_tracker");

		// OpenVR Space Calibrator groups devices into playspaces using the tracking-system name.
		// Body trackers should live with the HMD's playspace, while shadow trackers should live
		// with the original controller they mirror.
		std::string trackingSystemName = getTrackingSystemName();
		if (!trackingSystemName.empty())
		{
			props->SetStringProperty(
				container, vr::Prop_TrackingSystemName_String, trackingSystemName.c_str());
		}

		// Reference HTC's input profile (minimal - just pose)
		props->SetStringProperty(
			container, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json");

		// Set device class
		props->SetInt32Property(
			container, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);

		// Initialize pose
		mLastPose = ControllerCommon::generateDisconnectedPose();

		mIsActive = true;

		if (mRole.has_value())
		{
			DriverLog("HandOfLesser body tracker activated: %s", 
					  bodyTrackerRoleToString(mRole.value()));
		}
		else
		{
			DriverLog("HandOfLesser shadow tracker activated: %s", mSerialNumber.c_str());
		}

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

	void EmulatedTrackerDriver::UpdatePose(const HOL::BodyTrackerPosePayload& payload)
	{
		// Build DriverPose_t from payload
		vr::DriverPose_t pose{};
		pose.qWorldFromDriverRotation.w = 1.0;
		pose.qWorldFromDriverRotation.x = 0.0;
		pose.qWorldFromDriverRotation.y = 0.0;
		pose.qWorldFromDriverRotation.z = 0.0;

		pose.qDriverFromHeadRotation.w = 1.0;
		pose.qDriverFromHeadRotation.x = 0.0;
		pose.qDriverFromHeadRotation.y = 0.0;
		pose.qDriverFromHeadRotation.z = 0.0;

		// Set position
		pose.vecPosition[0] = payload.location.position.x();
		pose.vecPosition[1] = payload.location.position.y();
		pose.vecPosition[2] = payload.location.position.z();

		// Set orientation
		pose.qRotation.w = payload.location.orientation.w();
		pose.qRotation.x = payload.location.orientation.x();
		pose.qRotation.y = payload.location.orientation.y();
		pose.qRotation.z = payload.location.orientation.z();

		// Set velocities
		pose.vecVelocity[0] = payload.velocity.linearVelocity.x();
		pose.vecVelocity[1] = payload.velocity.linearVelocity.y();
		pose.vecVelocity[2] = payload.velocity.linearVelocity.z();

		pose.vecAngularVelocity[0] = payload.velocity.angularVelocity.x();
		pose.vecAngularVelocity[1] = payload.velocity.angularVelocity.y();
		pose.vecAngularVelocity[2] = payload.velocity.angularVelocity.z();

		// Body tracking often provides valid estimated poses without the TRACKED bit for
		// torso/arm joints. Treat valid poses as running so apps like VRChat don't
		// discard them while SteamVR still renders them.
		pose.poseIsValid = payload.valid;
		pose.deviceIsConnected = mDeviceConnected;
		pose.result = payload.valid ? vr::TrackingResult_Running_OK
								   : vr::TrackingResult_Running_OutOfRange;
		pose.poseTimeOffset = 0.0;

		UpdatePose(pose);
	}

	void EmulatedTrackerDriver::UpdatePose(const vr::DriverPose_t& pose)
	{
		mLastPose = pose;
		// Ensure connected state is preserved
		mLastPose.deviceIsConnected = mDeviceConnected;
		mPendingPoseUpdate = true;
	}

	void EmulatedTrackerDriver::SubmitPose()
	{
		if (mIsActive && mDeviceConnected && mDeviceIndex != vr::k_unTrackedDeviceIndexInvalid)
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(
				mDeviceIndex, GetPose(), sizeof(vr::DriverPose_t));
		}
	}

	void EmulatedTrackerDriver::FlushPoseUpdate()
	{
		if (!mPendingPoseUpdate || !mIsActive || mDeviceIndex == vr::k_unTrackedDeviceIndexInvalid)
		{
			return;
		}

		mPendingPoseUpdate = false;

		if (mDeviceConnected)
		{
			SubmitPose();
			return;
		}

		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(
			mDeviceIndex, ControllerCommon::generateDisconnectedPose(), sizeof(vr::DriverPose_t));
	}

	void EmulatedTrackerDriver::setConnectedState(bool connected)
	{
		if (mDeviceConnected == connected)
			return;

		mDeviceConnected = connected;
		mPendingPoseUpdate = true;
	}

} // namespace HOL
