#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"
#include "above_below_curl_plane_gesture.h"
#include "open_hand_pinch_gesture.h"

using namespace HOL;
using namespace HOL::OpenXR;

namespace HOL::Gesture::OpenHandPinchGesture
{
	float Gesture::evaluateInternal(GestureData data)
	{
		float proximity = this->mProxGesture->evaluate(data);

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

	void Gesture::setup()
	{
		this->name = "OpenHandPinchGesture";
		this->mSubGestures.clear();

		////////////////////
		// Pinch proximity
		////////////////////

		this->mProxGesture = ProximityGesture::Create();

		this->mProxGesture->setup(this->parameters.pinchFinger, this->parameters.side);

		this->mSubGestures.push_back(this->mProxGesture);

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
			if (otherFinger == this->parameters.pinchFinger)
				continue;

			auto gesture = AboveBelowCurlPlaneGesture::Gesture::Create();
			gesture->parameters.otherFinger = otherFinger;
			gesture->parameters.planeFinger = this->parameters.pinchFinger;
			gesture->parameters.side = this->parameters.side;

			this->mCurlPlaneGestures.push_back(gesture);
		}

		for (auto& gesture : this->mCurlPlaneGestures)
		{
			this->mSubGestures.push_back(gesture);
		}
	}
}

