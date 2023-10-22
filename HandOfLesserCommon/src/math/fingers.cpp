#include "fingers.h"

#include <numbers>

#include <Eigen/Core>
#include <Eigen/Geometry>

float computeAngleBetweenVectors(const Eigen::Vector3f& a, const Eigen::Vector3f& b)
{
	const float dotProduct = a.dot(b);
	const float length = a.norm() * b.norm();

	return acos(dotProduct / length);
}

float mapCurlToSteamVR(float curlInRadians, float maxCurlRadians = std::numbers::pi_v<float>)
{
	return std::min(1.0f, std::max(0.0f, curlInRadians / maxCurlRadians));
}

float computeOpenXrCurl(
	const Eigen::Vector3f& metacarpal,
	const Eigen::Vector3f& proximal,
	const Eigen::Vector3f& intermediate,
	const Eigen::Vector3f& distal,
	const Eigen::Vector3f& tip
)
{
	const Eigen::Vector3f vecMetacarpal = proximal - metacarpal;
	const Eigen::Vector3f vecProximal = intermediate - proximal;
	const Eigen::Vector3f vecIntermediate = distal - intermediate;
	const Eigen::Vector3f vecDistal = tip - distal;

	const float theta1 = computeAngleBetweenVectors(vecMetacarpal, vecProximal);
	const float theta2 = computeAngleBetweenVectors(vecProximal, vecIntermediate);
	const float theta3 = computeAngleBetweenVectors(vecIntermediate, vecDistal);

	const float curl = theta1 + theta2 + theta3;

	return mapCurlToSteamVR(curl);
}

Eigen::Vector3f computeOpenXrNormal(
	const Eigen::Vector3f& metacarpal,
	const Eigen::Vector3f& proximal,
	const Eigen::Vector3f& intermediate
)
{
	const Eigen::Vector3f vecMetacarpal = proximal - metacarpal;
	const Eigen::Vector3f vecProximal = intermediate - proximal;

	const Eigen::Vector3f normal = vecMetacarpal.cross(vecProximal).normalized();

	return normal;
}

float computeOpenXrSplay(const Eigen::Vector3f& normal1, const Eigen::Vector3f& normal2)
{
	const float splay = computeAngleBetweenVectors(normal1, normal2);

	return splay;
}
