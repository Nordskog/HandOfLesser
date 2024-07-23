#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <vector>
#include <functional>
#include <HandOfLesserCommon.h>
#include <memory>
#include <src/hands/hand_pose.h>

namespace HOL::Gesture
{
	struct GestureData
	{
		XrHandJointLocationEXT* joints[HandSide::HandSide_MAX];
		XrHandTrackingAimStateFB* aimState[HandSide::HandSide_MAX];
		HandPose* handPose[HandSide::HandSide_MAX];
		XrPosef HMDPose;
	};

	namespace BaseGesture 
	{
		class Gesture
		{
		public:
			Gesture(){};
			static std::shared_ptr<Gesture> Create()
			{
				return std::make_shared<Gesture>();
			}

			float evaluate(GestureData data);

			std::vector<std::shared_ptr<Gesture>>& getSubGestures();

			float lastValue = 0;
			std::string name = "baseGesture";
			std::vector<std::shared_ptr<Gesture>> mSubGestures;

		protected:
			virtual float evaluateInternal(GestureData data)
			{
				return 0;
			}

			// Some kind of map? 
			//virtual setup();
			
			// Call after populating 
			virtual void init(){};
		};
	}

} // namespace HOL
