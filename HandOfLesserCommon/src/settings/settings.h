#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <src/controller/controller.h>

namespace HOL
{
	namespace settings
	{
		struct FingerBendSettings
		{
			// clang-format off
			float CommonCurlCenter[3] = {
				50.3f, 
				44.7f,
				27.0f
			};

			float ThumbCurlCenter[3] = {
				20.1f, 
				-63.1f, 
				50.2f
			};

			float FingerSplayCenter[5] = {
				2.8f,	// Index
				-3.6f,
				12.9f, 
				16.2f, 
				-18.2f		// Thumb
			};

			// Range values must match unity!
			MotionRange CommonCurlRange[3] = {
				{-1.3, 1}, 
				{-1, 1}, 
				{-1, 1}
			};
			MotionRange ThumbCurlRange[3] = {
				{-4.1f, 0.4	}, 
				{-0.5, 1.0	},
				{-2, 1.27	}
			};

			MotionRange FingersplayRange[5] = {
				{-2,	1	},	// Index
				{-2,	1	},	// middle	
				{-3,	1	},	// ring
				{-3,	1.5f},	// little
				{-2,	1.5f}	// thumb
			};

			// clang-format on

			Eigen::Vector3f ThumbAxisOffset = Eigen::Vector3f(0, 0, -89);
		};

		struct SkeletalBendSettings
		{
			// clang-format off
			// clang-format off
			float CommonCurlCenter[3] = {
				7.0f, 
				0.0f,
				0.0f
			};

			float ThumbCurlCenter[3] = {
				-21.5f, 
				-34.6f, 
				2.5f
			};

			float FingerSplayCenter[5] = {
				-2.5f,// Index
				0.0f,
				-4.5f,
				-4.0f,
				-5.5f		// Thumb
			};


			// Range values must match unity!
			MotionRange CommonCurlRange[3] = {
				{30.f,  -100.f }, 
				{30.f,  -100.f }, 
				{30.f,  -100.f }
			};
			MotionRange ThumbCurlRange[3] = {
				{	75,	 -25.f		},
				{	0,	-100.f   },			
				{	25, -90.f	},
			};
			MotionRange FingersplayRange[5] = {
				{-25.0f,  60.0f  }, // index
				{-25.0f,  60.0f  },
				{ -60.0f, 25.0f  },
				{ -60.0f, 25.0f  },
				{-40.0f,  40.0f   } // thumb
			};

			// clang-format on
		};

		struct GeneralSettings
		{
			int MotionPredictionMS = 15; // ms
			int UpdateIntervalMS = 1;
			float steamPoseTimeOffset = .0f;
			float linearVelocityMultiplier = 0.f;
			float angularVelocityMultiplier = 0.f;
			bool forceInactive = false;
		};

		struct HandPoseSettings
		{
			HOL::ControllerMode mControllerMode = HOL::ControllerMode::HookedControllerMode;
			HOL::ControllerType mControllerType = HOL::ControllerType::OculusTouch_VDXR;
			Eigen::Vector3f OrientationOffset = Eigen::Vector3f(0, 0, 0);
			Eigen::Vector3f PositionOffset = Eigen::Vector3f(0, 0, 0);
		};

		struct VRChatSettings
		{
			bool sendFull = false;
			bool sendAlternating = false;
			bool sendPacked = true;
			
			bool interlacePacked = true;

			// Should always be 100 except for testing
			int packedUpdateInterval = 100;

			// Modify splay to work with humanoid rig
			bool useUnityHumanoidSplay = true;
			// Values we can adjust to send curl/spray to vrchat
			// without having to be in VR
			bool sendDebugOsc = false;
			bool alternateCurlTest = false;	// flip between -1 and 1 every 100ms
			float curlDebug = 0;
			float splayDebug = 0;
		};

		struct VisualizerSettings
		{
			bool followLeftHand;
			bool followRightHand;
		};

		struct DebugSettings
		{
	
		};

		struct InputSettings
		{
			bool sendOscInput = true;
			bool sendSteamVRInput = true;
			bool blockControllerInputWhileHandTracking = true;
		};

		struct HandOfLesserSettings
		{
			GeneralSettings general;
			FingerBendSettings fingerBend;
			SkeletalBendSettings skeletalBend;
			HandPoseSettings handPose;
			VRChatSettings vrchat;
			DebugSettings debug;
			VisualizerSettings visualizer;
			InputSettings input;
		};
	}
} // namespace HOL::settings