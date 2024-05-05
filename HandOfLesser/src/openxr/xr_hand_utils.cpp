
#include "xr_hand_utils.h"
#include "XrUtils.h"

namespace HOL::OpenXR
{
	XrHandJointLocationEXT& getJoint(XrHandJointLocationEXT leftHandJoints[],
									 XrHandJointLocationEXT rightHandJoints[],
									 XrHandJointEXT joint,
									 HOL::HandSide side)
	{
		return (side == HOL::HandSide::LeftHand ? leftHandJoints[joint] : rightHandJoints[joint]);
	}
	XrHandJointLocationEXT& getJoint(XrHandJointLocationEXT joints[], XrHandJointEXT joint)
	{
		return joints[joint];
	}

	Eigen::Vector3f getJointPosition(XrHandJointLocationEXT leftHandJoints[],
									 XrHandJointLocationEXT rightHandJoints[],
									 XrHandJointEXT joint,
									 HOL::HandSide side)
	{
		return HOL::OpenXR::toEigenVector(
			(side == HOL::HandSide::LeftHand ? leftHandJoints[joint] : rightHandJoints[joint])
				.pose.position);
	}

	Eigen::Vector3f getJointPosition(XrHandJointLocationEXT joints[], XrHandJointEXT joint)
	{
		return HOL::OpenXR::toEigenVector(joints[joint].pose.position);
	}

	Eigen::Vector3f getJointPosition(XrHandJointLocationEXT joint)
	{
		return HOL::OpenXR::toEigenVector(joint.pose.position);
	}

	Eigen::Quaternionf getJointOrientation(XrHandJointLocationEXT leftHandJoints[],
										   XrHandJointLocationEXT rightHandJoints[],
										   XrHandJointEXT joint,
										   HOL::HandSide side)
	{
		return HOL::OpenXR::toEigenQuaternion(
			(side == HOL::HandSide::LeftHand ? leftHandJoints[joint] : rightHandJoints[joint])
				.pose.orientation);
	}

	Eigen::Quaternionf getJointOrientation(XrHandJointLocationEXT joints[], XrHandJointEXT joint)
	{
		return HOL::OpenXR::toEigenQuaternion(joints[joint].pose.orientation);
	}

	Eigen::Quaternionf getJointOrientation(XrHandJointLocationEXT joint)
	{
		return HOL::OpenXR::toEigenQuaternion(joint.pose.orientation);
	}

	XrHandJointEXT getRootJoint(HOL::FingerType fingerType)
	{
		switch (fingerType)
		{
			case FingerType::FingerThumb:
				return XrHandJointEXT::XR_HAND_JOINT_WRIST_EXT; // Thumb joint calculated from wrist
			case FingerType::FingerIndex:
				return XrHandJointEXT::XR_HAND_JOINT_INDEX_METACARPAL_EXT;
			case FingerType::FingerMiddle:
				return XrHandJointEXT::XR_HAND_JOINT_MIDDLE_METACARPAL_EXT;
			case FingerType::FingerRing:
				return XrHandJointEXT::XR_HAND_JOINT_RING_METACARPAL_EXT;
			case FingerType::FingerLittle:
				return XrHandJointEXT::XR_HAND_JOINT_LITTLE_METACARPAL_EXT;
		}
	}

	XrHandJointEXT getFirstFingerJoint(HOL::FingerType fingerType)
	{
		switch (fingerType)
		{
			case FingerType::FingerThumb:
				return XrHandJointEXT::XR_HAND_JOINT_THUMB_METACARPAL_EXT;
			case FingerType::FingerIndex:
				return XrHandJointEXT::XR_HAND_JOINT_INDEX_PROXIMAL_EXT;
			case FingerType::FingerMiddle:
				return XrHandJointEXT::XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT;
			case FingerType::FingerRing:
				return XrHandJointEXT::XR_HAND_JOINT_RING_PROXIMAL_EXT;
			case FingerType::FingerLittle:
				return XrHandJointEXT::XR_HAND_JOINT_LITTLE_PROXIMAL_EXT;
		}
	}

	XrHandJointEXT getSecondFingerJoint(HOL::FingerType fingerType)
	{
		switch (fingerType)
		{
			return (XrHandJointEXT)(getFirstFingerJoint(fingerType) + 1);
		}
	}

	XrHandJointEXT getFingerTip(HOL::FingerType fingerType)
	{
		switch (fingerType)
		{
			case FingerType::FingerThumb:
				return XrHandJointEXT::XR_HAND_JOINT_THUMB_TIP_EXT;
			case FingerType::FingerIndex:
				return XrHandJointEXT::XR_HAND_JOINT_INDEX_TIP_EXT;
			case FingerType::FingerMiddle:
				return XrHandJointEXT::XR_HAND_JOINT_MIDDLE_TIP_EXT;
			case FingerType::FingerRing:
				return XrHandJointEXT::XR_HAND_JOINT_RING_TIP_EXT;
			case FingerType::FingerLittle:
				return XrHandJointEXT::XR_HAND_JOINT_LITTLE_TIP_EXT;
		}
	}

} // namespace HOL::OpenXR
