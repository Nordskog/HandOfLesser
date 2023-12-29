#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	// Note that this does NOT match XrHandEXT, because it begins at 1 rather than 0.
	// Use HOL::HandSide wherever possible, and only convert to XrHandEXT when necessary
	enum HandSide
	{
		LeftHand, RightHand, HandSide_MAX
	};

	enum FingerType
	{
		FingerIndex,
		FingerMiddle,
		FingerRing,
		FingerLittle,
		FingerThumb,
		FingerType_MAX
	};

	enum FingerBendType
	{
		CurlFirst,
		CurlSecond,
		CurlThird,
		Splay,
		FingerBendType_MAX
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
