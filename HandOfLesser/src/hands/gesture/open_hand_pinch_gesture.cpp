#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"
#include "above_below_curl_plane_gesture.h"
#include "open_hand_pinch_gesture.h"

using namespace HOL;
using namespace HOL::OpenXR;

float HOL::OpenHandPinchGesture::evaluateInternal(GestureData data)
{
	float proximity = this->mAimStateGesture->evaluate(data);

	// In this case, all the other fingers need to be above the plane.
	bool above = true;
	for (auto& planeGesture : this->mCurlPlaneGestures)
	{
		above = above && (planeGesture->evaluate(data) > 0);
	}

	// printf("Prox: %.3f, above: %s\n", proximity, (above ? "true" : "false"));

	if (above)
	{
		return proximity;
	}
	else
	{
		return 0;
	}
}

void HOL::OpenHandPinchGesture::setup(HOL::FingerType pinchFinger, HOL::HandSide side)
{
	this->mPinchFinger = pinchFinger;
	this->mSide = side;

	this->name = "OpenHandPinchGesture";
	this->mSubGestures.clear();

	////////////////////
	// Pinch proximity
	////////////////////

	this->mAimStateGesture = AimStateGesture::Create();

	this->mAimStateGesture->setup(pinchFinger, side);

	this->mSubGestures.push_back(this->mAimStateGesture);

	/////////////////////////////////////
	// Other fingers above pinch plane
	/////////////////////////////////////

	std::vector<HOL::FingerType> allPinchFingers = {FingerType::FingerIndex,
													FingerType::FingerMiddle,
													FingerType::FingerRing,
													FingerType::FingerLittle};

	for (auto otherFinger : allPinchFingers)
	{
		// Don't check the finger we are pinching
		if (otherFinger == pinchFinger)
			continue;

		auto gesture = AboveBelowCurlPlaneGesture::Create();
		gesture.get()->setup(mPinchFinger, otherFinger, side);

		this->mCurlPlaneGestures.push_back(gesture);
	}

	for (auto& gesture : this->mCurlPlaneGestures)
	{
		this->mSubGestures.push_back(gesture);
	}
}
