#pragma once

#include "settings_global.h"

namespace HOL
{
	namespace settings
	{
		float CommonCurlCenter[3] = {30.5f, 45.f, 20.6f};
		float ThumbCurlCenter[3] = {22, 22, 22};
		float FingerSplayCenter[5] = {7.5f, 1.8f, 7.3f, -7.3f, 0.f};
		Eigen::Vector3f ThumbAxisOffset(0, 0, 0);

		int MotionPredictionMS = 15; // ms
		Eigen::Vector3f OrientationOffset(0, 0, 0);
		Eigen::Vector3f PositionOffset(0, 0, 0);

		extern bool sendFull = false;
		extern bool sendAlternating = false;
		extern bool sendPacked = true;
	} // namespace settings
} // namespace HOL
