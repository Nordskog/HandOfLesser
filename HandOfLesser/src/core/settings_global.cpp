#pragma once

#include "settings_global.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	namespace settings
	{
		// clang-format off
		float CommonCurlCenter[3] = {
			38.6f, 
			45.f,
			25.f
		};

		float ThumbCurlCenter[3] = {
			18.8f, 
			2.0f, 
			47.6f
		};

		float FingerSplayCenter[5] = {
			2.8f,	// Index
			-3.6f,
			12.9f, 
			16.2f, 
			-24.0f		// Thumb
		};

		// Range values must match unity!
		MotionRange CommonCurlRange[3] = {
			{-1.2, 1}, 
			{-1, 1}, 
			{-1, 1}
		};
		MotionRange ThumbCurlRange[3] = {
			{-4.1f, 0.4	}, 
			{-1.1, 1.7	},
			{-1.7, 1.27	}
		};

		MotionRange FingersplayRange[5] = {
			{-1.5,	1	},	// Index
			{-2,	1	},		
			{-3,	1	}, 
			{-3,	1	},
			{-2,	1.5f}	// thumb
		};

		// clang-format on

		Eigen::Vector3f ThumbAxisOffset(0,0,-77);

		int MotionPredictionMS = 0; // ms
		int UpdateIntervalMS = 5;

		Eigen::Vector3f PositionOffset(0, 0, 0);
		Eigen::Vector3f OrientationOffset(0, 0, 0);

		bool sendFull = false;
		bool sendAlternating = false;
		bool sendPacked = true;

		// Modify splay to work with humanoid rig
		bool useUnityHumanoidSplay = true;

		void restoreDefaultControllerOffset(ControllerType type)
		{
			PoseLocationEuler def = HOL::getDefaultControllerOffset(type);

			PositionOffset = def.position;
			OrientationOffset = def.orientation;
		}

	} // namespace settings
} // namespace HOL
