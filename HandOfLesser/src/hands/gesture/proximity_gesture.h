#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL::Gesture
{
	class ProximityGesture : public BaseGesture::Gesture
	{

	public:
		ProximityGesture() : BaseGesture::Gesture(){};
		static std::shared_ptr<ProximityGesture> Create()
		{
			return std::make_shared<ProximityGesture>();
		}

		void setup(HOL::FingerType fingerTip1,
				   HOL::HandSide side1,
				   HOL::FingerType fingerTip2,
				   HOL::HandSide side2,
				   float minDistance = 0.025f,
				   float maxDistance = 0.08f);

		void setup(XrHandJointEXT joint1,
				   HOL::HandSide side1,
				   XrHandJointEXT joint2,
				   HOL::HandSide side2,
				   float minDistance = 0.025f,
				   float maxDistance = 0.08f);

		void setup(HOL::FingerType fingerTip1, HOL::HandSide side1);

	private:
		XrHandJointEXT mJoint1;
		XrHandJointEXT mJoint1Start;
		HOL::HandSide mSide1;
		XrHandJointEXT mJoint2;
		XrHandJointEXT mJoint2Start;
		HOL::HandSide mSide2;
		bool mUseLineDistance = false;

		float mMinDistance;
		float mMaxDistance;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
