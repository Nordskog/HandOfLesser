#include "utils.h"

#include <Eigen/Core>
#include <openxr/openxr_platform.h>

namespace HOL
{

Eigen::Vector3f toEigenVector(const XrVector3f& xrVector)
{
	return Eigen::Vector3f(xrVector.x, xrVector.y, xrVector.z);
}

Eigen::Quaternionf toEigenQuaternion(const XrQuaternionf& xrQuat)
{
	return Eigen::Quaternionf(xrQuat.w, xrQuat.x, xrQuat.y, xrQuat.z);
}

} // namespace HOL
