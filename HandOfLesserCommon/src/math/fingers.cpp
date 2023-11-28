#include "fingers.h"

#include <numbers>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	float computeAngleBetweenVectors(const Eigen::Vector3f& a, const Eigen::Vector3f& b)
	{
		const float dotProduct = a.dot(b);
		const float length = a.norm() * b.norm();

		return acos(dotProduct / length);
	}

	float computeCurl(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next)
	{
		Eigen::Quaternionf localRot = previous.inverse() * next;
		Eigen::Vector3f euler = localRot.toRotationMatrix().canonicalEulerAngles(0, 1, 2);
		return euler.x();
	}

	float computeSplay(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next)
	{
		Eigen::Quaternionf localRot = previous.inverse() * next;
		Eigen::Vector3f euler = localRot.toRotationMatrix().canonicalEulerAngles(0, 1, 2);
		return euler.y();
	}

	float mapCurlToSteamVR(float curlInRadians, float maxCurlRadians)
	{
		return std::min(1.0f, std::max(0.0f, curlInRadians / maxCurlRadians));
	}
} // namespace HOL
