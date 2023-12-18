#pragma once

#include "settings_global.h"

namespace HOL
{
	namespace settings
	{
		extern int MotionPredictionMS = 15;	// ms
		Eigen::Vector3f OrientationOffset(0, 0, 0);
		Eigen::Vector3f PositionOffset(0, 0, 0);
	} // namespace settings
} // namespace HOL
