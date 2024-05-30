#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"
#include "above_below_curl_plane_gesture.h"
#include "src/core/ui/user_interface.h"

using namespace HOL;
using namespace HOL::OpenXR;

float AboveBelowCurlPlaneGesture::evaluateInternal(GestureData data)
{
	// The plane is defined by the X axis of the palm, and the vector between the knucle
	// and the tip of the finger.
	auto planeKnuckleJoint
		= getJointPosition(data.joints[this->mSide], getFirstFingerJoint(this->mPlaneFinger));
	auto planeTipJoint = getJointPosition(data.joints[this->mSide],
										  (XrHandJointEXT)(getFingerTip(this->mPlaneFinger)));

	auto palmOrientation
		= getJointOrientation(data.joints[this->mSide], XrHandJointEXT::XR_HAND_JOINT_PALM_EXT);

	// Palm X and knucke-to-tip crossed to get plane orientation
	Eigen::Vector3f palmX = palmOrientation * Eigen::Vector3f(1, 0, 0);

	Eigen::Vector3f planeNormal = palmX.cross(planeTipJoint - planeKnuckleJoint);
	planeNormal.normalize();

	// dot product of normal and vector from plane tip to other tip decide over/under.
	auto otherTipJoint
		= getJointPosition(data.joints[this->mSide], getFingerTip(this->mOtherFinger));

	Eigen::Vector3f otherVector = otherTipJoint - planeTipJoint;

	return planeNormal.dot(otherVector);
}

void HOL::AboveBelowCurlPlaneGesture::setup(HOL::FingerType planeFinger,
											HOL::FingerType otherFinger,
											HOL::HandSide side)
{
	this->name = "AboveBelowCurlPlaneGesture";
	this->mPlaneFinger = planeFinger;
	this->mOtherFinger = otherFinger;
	this->mSide = side;
}
