#include "controller.h"
namespace HOL
{
	PoseLocationEuler getControllerBaseOffset()
	{
		return {Eigen::Vector3f(0.077f, -0.042f, -0.106f),
				Eigen::Vector3f(-0.300f, -40.510f, -89.296f)};
	}

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type)
	{
		switch (type)
		{
			case ControllerOffsetPreset::RoughyVRChatHand: {
				return {Eigen::Vector3f(0.0015, -0.001f, 0),
						Eigen::Vector3f(-4.0f, 0, 3.0f)};
			}

			case ControllerOffsetPreset::ZERO: {
				return {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)};
			}
		}
	}

	const char* bodyTrackerRoleToString(BodyTrackerRole role)
	{
		switch (role)
		{
			case BodyTrackerRole::Hips: return "Hips";
			case BodyTrackerRole::Chest: return "Chest";
			case BodyTrackerRole::LeftUpperArm: return "Left Upper Arm";
			case BodyTrackerRole::LeftLowerArm: return "Left Lower Arm";
			case BodyTrackerRole::RightUpperArm: return "Right Upper Arm";
			case BodyTrackerRole::RightLowerArm: return "Right Lower Arm";
			default: return "Unknown";
		}
	}

	const char* bodyTrackerRoleToSerial(BodyTrackerRole role)
	{
		switch (role)
		{
			case BodyTrackerRole::Hips: return "HOL_hips";
			case BodyTrackerRole::Chest: return "HOL_chest";
			case BodyTrackerRole::LeftUpperArm: return "HOL_left_upper_arm";
			case BodyTrackerRole::LeftLowerArm: return "HOL_left_lower_arm";
			case BodyTrackerRole::RightUpperArm: return "HOL_right_upper_arm";
			case BodyTrackerRole::RightLowerArm: return "HOL_right_lower_arm";
			default: return "HOL_unknown";
		}
	}

	XrBodyJointFB bodyTrackerRoleToJoint(BodyTrackerRole role)
	{
		switch (role)
		{
			case BodyTrackerRole::Hips: return XR_BODY_JOINT_HIPS_FB;
			case BodyTrackerRole::Chest: return XR_BODY_JOINT_CHEST_FB;
			case BodyTrackerRole::LeftUpperArm: return XR_BODY_JOINT_LEFT_ARM_UPPER_FB;
			case BodyTrackerRole::LeftLowerArm: return XR_BODY_JOINT_LEFT_ARM_LOWER_FB;
			case BodyTrackerRole::RightUpperArm: return XR_BODY_JOINT_RIGHT_ARM_UPPER_FB;
			case BodyTrackerRole::RightLowerArm: return XR_BODY_JOINT_RIGHT_ARM_LOWER_FB;
			default: return XR_BODY_JOINT_HIPS_FB; // Fallback
		}
	}

	const char* bodyTrackerRoleToTrackerRoleString(BodyTrackerRole role)
	{
		switch (role)
		{
			case BodyTrackerRole::Hips: return "TrackerRole_Waist";
			case BodyTrackerRole::Chest: return "TrackerRole_Chest";
			case BodyTrackerRole::LeftUpperArm: return "TrackerRole_LeftShoulder";
			case BodyTrackerRole::LeftLowerArm: return "TrackerRole_LeftElbow";
			case BodyTrackerRole::RightUpperArm: return "TrackerRole_RightShoulder";
			case BodyTrackerRole::RightLowerArm: return "TrackerRole_RightElbow";
			default: return "TrackerRole_Handed";
		}
	}

} // namespace HOL
