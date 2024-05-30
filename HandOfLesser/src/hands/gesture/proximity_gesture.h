#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	class ProximityGesture : public BaseGesture
	{

	public:
		ProximityGesture() : BaseGesture(){};
		static std::shared_ptr<ProximityGesture> Create()
		{
			return std::make_shared<ProximityGesture>();
		}

		void setup(HOL::FingerType fingerTip1,
				   HOL::HandSide side1,
				   HOL::FingerType fingerTip2,
				   HOL::HandSide side2,
				   float minDistance = 0.025f,
				   float maxDistance = 1.0f);

		void setup(XrHandJointEXT joint1,
				   HOL::HandSide side1,
				   XrHandJointEXT joint2,
				   HOL::HandSide side2,
				   float minDistance = 0.0250f,
				   float maxDistance = 1.0f);

		void setup(HOL::FingerType fingerTip1, HOL::HandSide side1);

	private:
		XrHandJointEXT mJoint1;
		HOL::HandSide mSide1;
		XrHandJointEXT mJoint2;
		HOL::HandSide mSide2;

		float mMinDistance;
		float mMaxDistance;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
