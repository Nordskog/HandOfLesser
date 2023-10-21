#include "math_utils.h"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

namespace HOL
{
Eigen::Quaternionf quaternionFromEulerAngles(float x, float y, float z)
{
	return Eigen::Quaternionf(
		Eigen::AngleAxisf(z, Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(y, Eigen::Vector3f::UnitY())
		* Eigen::AngleAxisf(x, Eigen::Vector3f::UnitX())
	);
}

Eigen::Quaternionf quaternionFromEulerAnglesDegrees(float x, float y, float z)
{
	return quaternionFromEulerAngles(degreesToRadians(x), degreesToRadians(y), degreesToRadians(z));
}

Eigen::Quaternionf quaternionFromEulerAnglesDegrees(Eigen::Vector3f degrees)
{
	return quaternionFromEulerAnglesDegrees(degrees.x(), degrees.y(), degrees.z());
}

float degreesToRadians(float degrees)
{
	return degrees * (M_PI / 180.0f);
}

Eigen::Vector3f flipHandRotation(Eigen::Vector3f& rot)
{
	// left/right rotation needs to flip x?
	return Eigen::Vector3f(rot.x(), rot.y(), -rot.z());
}

Eigen::Vector3f flipHandTranslation(Eigen::Vector3f& trans)
{
	// Left/right translation only needs to flip z?
	return Eigen::Vector3f(-trans.x(), trans.y(), trans.z());
}

Eigen::Vector3f toEigenVector(XrVector3f& xrVector)
{
	return Eigen::Vector3f(xrVector.x, xrVector.y, xrVector.z);
}

Eigen::Quaternionf toEigenQuaternion(XrQuaternionf& xrQuat)
{
	return Eigen::Quaternionf(xrQuat.w, xrQuat.x, xrQuat.y, xrQuat.z);
}

} // namespace HOL
