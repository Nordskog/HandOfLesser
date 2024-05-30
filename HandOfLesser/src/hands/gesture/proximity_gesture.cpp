#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"

using namespace HOL;

float ProximityGesture::evaluateInternal(GestureData data)
{
	auto pos1 = OpenXR::getJointPosition(data.joints[this->mSide1], this->mJoint1);
	auto pos2 = OpenXR::getJointPosition(data.joints[this->mSide2], this->mJoint2);

	auto vectorBetween = pos1 - pos2;

	float distance = vectorBetween.norm();

	distance -= this->mMinDistance;
	distance = std::clamp(distance, 0.0f, this->mMaxDistance);
	distance /= (this->mMaxDistance - this->mMinDistance);
	distance = 1.f - distance;

	return distance;
}

void HOL::ProximityGesture::setup(HOL::FingerType fingerTip1,
								  HOL::HandSide side1,
								  HOL::FingerType fingerTip2,
								  HOL::HandSide side2,
								  float minDistance,
								  float maxDistance)
{
	auto joint1 = OpenXR::getFingerTip(fingerTip1);
	auto joint2 = OpenXR::getFingerTip(fingerTip2);
	setup(joint1, side1, joint2, side2, minDistance, maxDistance);
}

void HOL::ProximityGesture::setup(XrHandJointEXT joint1,
								  HOL::HandSide side1,
								  XrHandJointEXT joint2,
								  HOL::HandSide side2,
								  float minDistance,
								  float maxDistance)
{
	this->name = "ProximityGesture";

	this->mJoint1 = joint1;
	this->mSide1 = side1;
	this->mJoint2 = joint2;
	this->mSide2 = side2;
	this->mMinDistance = minDistance;
	this->mMaxDistance = maxDistance;
}

void HOL::ProximityGesture::setup(HOL::FingerType fingerTip1, HOL::HandSide side1)
{
	auto joint1 = OpenXR::getFingerTip(fingerTip1);
	auto joint2 = OpenXR::getFingerTip(HOL::FingerType::FingerThumb);
	setup(joint1, side1, joint2, side1);
}
