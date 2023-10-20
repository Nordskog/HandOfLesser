#pragma once 

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <d3d11.h> 
#include <openxr/openxr_platform.h>
#include "openxr/openxr_structs.hpp"

namespace HOL
{
	Eigen::Quaternionf  quaternionFromEulerAngles(Eigen::Vector3f);
	Eigen::Quaternionf  quaternionFromEulerAnglesDegrees(Eigen::Vector3f degrees);
	Eigen::Quaternionf  quaternionFromEulerAnglesDegrees(float x, float y, float z);
	Eigen::Quaternionf  quaternionFromEulerAngles(float x, float y, float z);
	float degreesToRadians(float degrees);

	Eigen::Vector3f flipHandRotation(Eigen::Vector3f& rot);
	Eigen::Vector3f flipHandTranslation(Eigen::Vector3f& trans);

	Eigen::Vector3f toEigenVector(XrVector3f& xrVector);
	Eigen::Quaternionf toEigenQuaternion(XrQuaternionf& xrQuat);
}


