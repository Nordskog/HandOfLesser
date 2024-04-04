#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>

namespace HOL
{
	namespace settings
	{
		struct FingerBendSettings
		{
			// clang-format off
			float CommonCurlCenter[3] = {
				38.6f, 
				45.f,
				25.f
			};

			float ThumbCurlCenter[3] = {
				20.1f, 
				-14.5f, 
				45.7f
			};

			float FingerSplayCenter[5] = {
				2.8f,	// Index
				-3.6f,
				12.9f, 
				16.2f, 
				-24.3f		// Thumb
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

			Eigen::Vector3f ThumbAxisOffset = Eigen::Vector3f(0, 0, -89);
		};

		struct GeneralSettings
		{
			int MotionPredictionMS = 0; // ms
			int UpdateIntervalMS = 5;
			float steamPoseTimeOffset = .0f;
		};

		struct HandPoseSettings
		{
			Eigen::Vector3f OrientationOffset = Eigen::Vector3f(0, 0, 0);
			Eigen::Vector3f PositionOffset = Eigen::Vector3f(0, 0, 0);
		};

		struct VRChatSettings
		{
			bool sendFull = false;
			bool sendAlternating = false;
			bool sendPacked = true;

			// Modify splay to work with humanoid rig
			bool useUnityHumanoidSplay = true;
		};

		struct HandOfLesserSettings
		{
			GeneralSettings general;
			FingerBendSettings fingerBend;
			HandPoseSettings handPose;
			VRChatSettings vrchat;
		};
	}
} // namespace HOL::settings