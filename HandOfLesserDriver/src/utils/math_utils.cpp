#include "math_utils.h"

Eigen::Vector3f HOL::ovrVectorToEigen(double vecPosition[3])
{
	return Eigen::Vector3f(vecPosition[0], vecPosition[1], vecPosition[2]);
}

Eigen::Quaternionf HOL::ovrQuaternionToEigen(vr::HmdQuaternion_t pose)
{
	return Eigen::Quaternionf(pose.w, pose.x, pose.y, pose.z);
}
