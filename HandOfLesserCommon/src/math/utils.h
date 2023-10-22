#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <d3d11.h>
#include <openxr/openxr_platform.h>

namespace HOL
{

Eigen::Vector3f toEigenVector(const XrVector3f& xrVector);
Eigen::Quaternionf toEigenQuaternion(const XrQuaternionf& xrQuat);

} // namespace HOL
