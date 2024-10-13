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
			float commonCurlCenter[3] = {
				50.3f, 
				44.7f,
				27.0f
			};

			float thumbCurlCenter[3] = {
				20.1f, 
				-2.1f, 
				50.2f
			};

			float fingerSplayCenter[5] = {
				2.8f,	// Index
				-3.6f,
				12.9f, 
				16.2f, 
				-18.2f		// Thumb
			};

			// Range values must match unity!
			MotionRange commonCurlRange[3] = {
				{-1.3, 1}, 
				{-1, 1}, 
				{-1, 1}
			};
			MotionRange thumbCurlRange[3] = {
				{-4.1f, 0.4	}, 
				{-0.5, 1.0	},
				{-2, 1.27	}
			};

			MotionRange fingersplayRange[5] = {
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
			float commonCurlCenter[3] = {
				7.0f, 
				0.0f,
				0.0f
			};

			float thumbCurlCenter[3] = {
				-21.5f, 
				-34.6f, 
				2.5f
			};

			float fingerSplayCenter[5] = {
				-2.5f,// Index
				0.0f,
				-4.5f,
				-4.0f,
				-5.5f		// Thumb
			};


			// Range values must match unity!
			MotionRange commonCurlRange[3] = {
				{30.f,  -100.f }, 
				{30.f,  -100.f }, 
				{30.f,  -100.f }
			};
			MotionRange thumbCurlRange[3] = {
				{	75,	 -25.f		},
				{	0,	-100.f   },			
				{	25, -90.f	},
			};
			MotionRange fingersplayRange[5] = {
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
			int motionPredictionMS = 15; // ms
			int updateIntervalMS = 1;
			float steamPoseTimeOffset = .0f;
			float linearVelocityMultiplier = 0.f;
			float angularVelocityMultiplier = 0.f;
			bool forceInactive = false;
			bool jitterLastPoseOnTrackingLoss = true;
		};

		struct HandPoseSettings
		{
			HOL::ControllerType controllerType = HOL::ControllerType::OculusTouch_VDXR;
			Eigen::Vector3f orientationOffset = Eigen::Vector3f(0, 0, 0);
			Eigen::Vector3f positionOffset = Eigen::Vector3f(0, 0, 0);
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

		struct SkeletalInput
		{
			float jointLengthMultiplier = 1.05f;
			bool sendSkeletalInput = true;
			Eigen::Vector3f positionOffset = Eigen::Vector3f(-0.168f, -0.041f, 0.049f);
			Eigen::Vector3f orientationOffset = Eigen::Vector3f(174.300f, 1.221f, 136.930f);
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
			SkeletalInput skeletal;
		};
	}
} // namespace HOL::settings