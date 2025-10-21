#pragma once

#include "generic_control_interface.h"
#include <openvr_driver.h>
#include "src/input/InputCommons.h"
#include "src/steamvr/skeletal_input_joints.h"

namespace HOL
{
	class HookedController : public GenericControllerInterface
	{
	public:
		HookedController(uint32_t id,
						 HandSide side,
						 vr::IVRServerDriverHost* host,
						 vr::ITrackedDeviceServerDriver* driver,
						 vr::PropertyContainerHandle_t propertyContainer);

		void lateInit(std::string serial,
					  vr::ETrackedDeviceClass deviceClass,
					  vr::ETrackedControllerRole role);

		void UpdatePose(HOL::HandTransformPacket* packet) override;
		void UpdateInput(HOL::ControllerInputPacket* packet) override;
		void UpdateBoolInput(const std::string& input, bool value) override;
		void UpdateFloatInput(const std::string& input, float value) override;
		void UpdateSkeletal(HOL::SkeletalPacket* packet) override;
		void SubmitPose() override;
		void registerSkeletonInput(vr::VRInputComponentHandle_t handle,
								   vr::EVRSkeletalTrackingLevel level,
								   const std::string& path);
		void clearAugmentedSkeleton();
		bool isAugmentedSkeletonActive() const;
		void resetPossessionHints();

		bool canPossess();
		bool shouldPossess();

		void setSide(HandSide side);
		HandSide getSide();
		void updateSideFromRole();

		void setLastOriginalPoseState(bool valid);
		bool isHeld();
		Eigen::Vector3f getWorldPosition();

		vr::PropertyContainerHandle_t propertyContainer = 0;
		vr::ETrackedControllerRole role = vr::ETrackedControllerRole::TrackedControllerRole_Max;
		vr::ETrackedDeviceClass mDeviceClass = vr::ETrackedDeviceClass::TrackedDeviceClass_Max;
		std::string serial;
		vr::IVRDriverInput* driverInput;
		std::unordered_map<vr::VRInputComponentHandle_t, ControllerInputHandle> inputHandles;
		std::unordered_map<std::string, vr::VRInputComponentHandle_t> inputHandlesByName;

		uint32_t getDeviceId();

		vr::DriverPose_t lastOriginalPose;
		bool mLastOriginalPoseValid;

		// Frames since last updated by original driver.
		// Reset every time there's an update, if we are possessing or offsetting.
		int32_t framesSinceLastPoseUpdate = 0;

	private:
		vr::DriverPose_t mLastPose;
		bool mLastHeldState
			= true; // Controllers not added until held, so should be true on activate

		HOL::HandTransformPacket mLastTransformPacket;
		HOL::ControllerInputPacket mLastInputPacket;

		bool mValidWhileOriginalInvalid;

		HandSide mSide;
		vr::IVRServerDriverHost* mHookedHost;
		vr::ITrackedDeviceServerDriver* mHookedDriver;
		uint32_t mDeviceId;

		// State tracking for logging
		bool mLastPossessionState = false;
		int32_t mFramesSinceLastDistanceLog = 0;
		vr::VRInputComponentHandle_t mSkeletonHandle = 0;
		vr::EVRSkeletalTrackingLevel mSkeletonTrackingLevel = vr::VRSkeletalTracking_Estimated;
		std::string mSkeletonInputPath;
		bool mLoggedMissingSkeletonHandle = false;
		vr::VRBoneTransform_t mSkeletalPose[SteamVR::HandSkeletonBone::eBone_Count]{};
	};
} // namespace HOL
