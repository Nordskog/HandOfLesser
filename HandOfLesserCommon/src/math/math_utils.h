#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{

	Eigen::Quaternionf quaternionFromEulerAngles(Eigen::Vector3f);
	Eigen::Quaternionf quaternionFromEulerAnglesDegrees(Eigen::Vector3f degrees);
	Eigen::Quaternionf quaternionFromEulerAnglesDegrees(float x, float y, float z);
	Eigen::Quaternionf quaternionFromEulerAngles(float x, float y, float z);
	float degreesToRadians(float degrees);
	float radiansToDegrees(float radians);

	Eigen::Vector3f flipHandRotation(Eigen::Vector3f& rot);
	Eigen::Vector3f flipHandTranslation(Eigen::Vector3f& trans);

} // namespace HOL
