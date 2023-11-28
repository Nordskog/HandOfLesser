#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <numbers>

namespace HOL
{
	float computeAngleBetweenVectors(const Eigen::Vector3f& a, const Eigen::Vector3f& b);

	float computeCurl(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next);
	float computeSplay(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next);

	float mapCurlToSteamVR(float curlInRadians, float maxCurlRadians = std::numbers::pi_v<float>);
} // namespace HOL
