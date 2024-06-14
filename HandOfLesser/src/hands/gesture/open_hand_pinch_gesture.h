#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>
#include <vector>
#include "aim_state_gesture.h"
#include "above_below_curl_plane_gesture.h"

namespace HOL
{
	class OpenHandPinchGesture : public BaseGesture
	{

	public:
		OpenHandPinchGesture() : BaseGesture(){};
		static std::shared_ptr<OpenHandPinchGesture> Create()
		{
			return std::make_shared<OpenHandPinchGesture>();
		}

		void setup(HOL::FingerType mPinchFinger, HOL::HandSide Side);

	private:
		HOL::FingerType mPinchFinger;
		HOL::HandSide mSide;

		std::vector<std::shared_ptr<AboveBelowCurlPlaneGesture>> mCurlPlaneGestures;
		std::shared_ptr<AimStateGesture> mAimStateGesture;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
