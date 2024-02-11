#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL::settings
{
	extern float CommonCurlCenter[3];  // Curl for 4 main fingers
	extern float ThumbCurlCenter[3];   // Curl for thumb
	extern float FingerSplayCenter[5]; // Splay for each finger
	extern Eigen::Vector3f ThumbAxisOffset;

	extern int MotionPredictionMS;
	extern Eigen::Vector3f OrientationOffset;
	extern Eigen::Vector3f PositionOffset;

	extern bool sendFull;
	extern bool sendAlternating;
	extern bool sendPacked;

	extern bool useUnityHumanoidSplay;
} // namespace HOL::settings