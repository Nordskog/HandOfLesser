#include "fingers.h"

#include <numbers>

#include "math_utils.h"

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

	std::pair<float, float> computeCurlSplay(const Eigen::Quaternionf& previous,
											 const Eigen::Quaternionf& next)
	{
		Eigen::Quaternionf localRot = previous.inverse() * next;

		Eigen::Vector3f prevForward = Eigen::Vector3f(0, 0, -1);
		Eigen::Vector3f nextForward = localRot * Eigen::Vector3f(0, 0, -1);
		Eigen::Vector3f xPlane = Eigen::Vector3f(1, 0, 0); // Plane defined by x axis

		// Project forward onto plane
		float planeDot = nextForward.dot(xPlane); // distance from plane
		Eigen::Vector3f forwardProjected = nextForward - (xPlane * planeDot);
		forwardProjected.normalize();

		// Both are now on the same plane
		// Treat as negative rotation if above 0
		float curl = computeAngleBetweenVectors(prevForward, forwardProjected);
		if (forwardProjected.y() > 0)
		{
			curl *= -1.f;
		}

		// Splay is the angle between original and projected
		// Treat as negative rotation if above 0
		float splay = computeAngleBetweenVectors(nextForward, forwardProjected);
		if (planeDot > 0)
		{
			splay *= -1.f;
		}

		return std::make_pair(curl, splay);
	}

	float mapCurlToSteamVR(float curlInRadians, float maxCurlRadians)
	{
		return std::min(1.0f, std::max(0.0f, curlInRadians / maxCurlRadians));
	}
} // namespace HOL
