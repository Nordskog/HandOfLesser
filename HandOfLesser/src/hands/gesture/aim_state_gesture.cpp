#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "aim_state_gesture.h"

using namespace HOL;

float AimStateGesture::evaluateInternal(GestureData data)
{
	auto aimStateData = data.aimState[this->mSide];
	switch (this->mFinger)
	{
		case FingerType::FingerIndex:
			return aimStateData->pinchStrengthIndex;
		case FingerType::FingerMiddle:
			return aimStateData->pinchStrengthMiddle;
		case FingerType::FingerRing:
			return aimStateData->pinchStrengthRing;
		case FingerType::FingerLittle:
			return aimStateData->pinchStrengthLittle;
		default:
			return 0;
	}
}

void HOL::AimStateGesture::setup(HOL::FingerType finger,
											HOL::HandSide side)
{
	this->name = "AimStateGesture";
	this->mFinger = finger;
	this->mSide = side;
}
