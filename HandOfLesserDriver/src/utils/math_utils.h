#pragma once

#include "openvr_driver.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	Eigen::Vector3f ovrVectorToEigen(double vecPosition[3]);
	Eigen::Quaternionf ovrQuaternionToEigen(vr::HmdQuaternion_t pose);



}