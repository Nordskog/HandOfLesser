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

	// Would return the actual roll of the joint ( z axie before curl ), but is unused 
	// at the moment because we're dealing with unity humanoid garbage instead.
	// This is probably not correct, though it looks vaguely correctish.
	float computeSplay(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next)
	{
		
		Eigen::Quaternionf localRot = previous.inverse() * next;
		Eigen::Vector3f euler = localRot.toRotationMatrix().canonicalEulerAngles(2, 1, 0);
		return euler.y();
	}

	float computeUncurledSplay(const Eigen::Quaternionf& previous, const Eigen::Quaternionf& next, float curl)
	{
		Eigen::Quaternionf localRot = previous.inverse() * next;

		// Un-curl the finger
		localRot = localRot * quaternionFromEulerAngles(Eigen::Vector3f(-curl, 0, 0));

		// Extract y rotation
		Eigen::Vector3f euler = localRot.toRotationMatrix().canonicalEulerAngles(0, 1, 2);
		return euler.y();
	}

	// Creates a plane defined by palmOrientation and knucklePosition, returns angle between
	// tip position and its closest point on the plane. This emulates how Humanoid spread works.
	float computeHumanoidSplay(const Eigen::Quaternionf& palmOrientation,
							   const Eigen::Vector3f& knuclePosition,
							   const Eigen::Vector3f& tipPosition)
	{
		Eigen::Vector3f tipLocal = palmOrientation.inverse() * (tipPosition - knuclePosition);
		Eigen::Vector3f xPlane = Eigen::Vector3f(1, 0, 0); // Plane defined by x axis

		// Project tip onto plane
		float planeDot = tipLocal.dot(xPlane); // distance from plane
		Eigen::Vector3f forwardProjected = tipLocal - (xPlane * planeDot);
		forwardProjected.normalize();	// Don't really need this do we

		// Splay is the angle between original and projected
		// Treat as negative rotation if above 0
		float splay = computeAngleBetweenVectors(forwardProjected, tipLocal);
		if (planeDot > 0)
		{
			splay *= -1.f;
		}

		return splay;
	}

	float mapCurlToSteamVR(float curlInRadians, float maxCurlRadians)
	{
		return std::min(1.0f, std::max(0.0f, curlInRadians / maxCurlRadians));
	}
} // namespace HOL
