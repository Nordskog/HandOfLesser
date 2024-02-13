#pragma once

#include "settings_global.h"

namespace HOL
{
	namespace settings
	{
		// clang-format off
		float CommonCurlCenter[3] = {
			35.7f, 
			45.f,
			25.f
		};

		float ThumbCurlCenter[3] = {
			22, 
			22, 
			22
		};

		float FingerSplayCenter[5] = {
			2.8f,	// Index
			-3.6f,
			12.9f, 
			16.2f, 
			0.f		// Thumb
		};

		// Range values must match unity!
		MotionRange CommonCurlRange[3] = {
			{-1.2, 1}, 
			{-1, 1}, 
			{-1, 1}
		};
		MotionRange ThumbCurlRange[3] = {
			{-3, 1}, 
			{-1, 1},
			{-1, 1}
		};

		MotionRange FingersplayRange[5] = {
			{-1.5,	1	},	// Index
			{-2,	1	},		
			{-3,	1	}, 
			{-3,	1	},
			{-2,	1.5f}	// thumb
		};

		// clang-format on

		Eigen::Vector3f ThumbAxisOffset(0, 0, 0);

		int MotionPredictionMS = 15; // ms
		Eigen::Vector3f OrientationOffset(0, 0, 0);
		Eigen::Vector3f PositionOffset(0, 0, 0);

		bool sendFull = false;
		bool sendFull = true;
		bool sendAlternating = false;
		bool sendPacked = true;
		bool sendPacked = false;

		// Modify splay to work with humanoid rig
		bool useUnityHumanoidSplay = true;
	} // namespace settings
} // namespace HOL
