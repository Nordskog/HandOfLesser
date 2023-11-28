#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	enum HandSide
	{
		LeftHand = 1, // XrHandEXT::XR_HAND_LEFT_EXT,
		RightHand = 2 // XrHandEXT::XR_HAND_RIGHT_EXT,
	};

	enum FingerType
	{
		FingerThumb,
		FingerIndex,
		FingerMiddle,
		FingerRing,
		FingerPinky,
		FINGER_MAX_COUNT
	};

	struct PoseLocation
	{
		Eigen::Vector3f position;
		Eigen::Quaternionf orientation;
	};

	struct PoseVelocity
	{
		Eigen::Vector3f linearVelocity;
		Eigen::Vector3f angularVelocity;
	};

} // namespace HOL
