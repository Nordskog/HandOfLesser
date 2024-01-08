#include "math_utils.h"
#include <numbers> 


namespace HOL
{
	Eigen::Quaternionf quaternionFromEulerAngles(float x, float y, float z)
	{
		return Eigen::Quaternionf(Eigen::AngleAxisf(z, Eigen::Vector3f::UnitZ())
								  * Eigen::AngleAxisf(y, Eigen::Vector3f::UnitY())
								  * Eigen::AngleAxisf(x, Eigen::Vector3f::UnitX()));
	}

	Eigen::Quaternionf quaternionFromEulerAnglesDegrees(float x, float y, float z)
	{
		return quaternionFromEulerAngles(
			degreesToRadians(x), degreesToRadians(y), degreesToRadians(z));
	}

	Eigen::Quaternionf quaternionFromEulerAnglesDegrees(Eigen::Vector3f degrees)
	{
		return quaternionFromEulerAnglesDegrees(degrees.x(), degrees.y(), degrees.z());
	}

	float degreesToRadians(float degrees)
	{
		return degrees * (std::numbers::pi_v<float> / 180.0f);
	}

	float radiansToDegrees(float radians)
	{
		return radians * (180.0f / std::numbers::pi_v<float>);
	}
	float angleBetweenVectors(const Eigen::Vector3f& first, const Eigen::Vector3f& second)
	{
		return std::atan2(first.cross(second).norm(), first.dot(second));
	}

	Eigen::Vector3f flipHandRotation(Eigen::Vector3f& rot)
	{
		// left/right rotation needs to flip x and y?
		return Eigen::Vector3f(rot.x(), -rot.y(), -rot.z());
	}

	Eigen::Vector3f flipHandTranslation(Eigen::Vector3f& trans)
	{
		// Left/right translation only needs to flip z?
		return Eigen::Vector3f(-trans.x(), trans.y(), trans.z());
	}

} // namespace HOL
