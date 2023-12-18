#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL::settings
{
	extern int MotionPredictionMS;
	extern Eigen::Vector3f OrientationOffset;
	extern Eigen::Vector3f PositionOffset;
} // namespace HOL::settings