#include "aim_gesture.h"

namespace HOL::Gesture::AimGesture
{
	float Gesture::evaluateInternal(GestureData data)
	{
		auto* aimState = data.aimState[this->parameters.side];
		if (aimState == nullptr)
		{
			return 0.0f;
		}

		if ((aimState->status & XR_HAND_TRACKING_AIM_VALID_BIT_FB)
			!= XR_HAND_TRACKING_AIM_VALID_BIT_FB)
		{
			return 0.0f;
		}

		return (aimState->status & this->parameters.flags) == this->parameters.flags ? 1.0f : 0.0f;
	}
} // namespace HOL::Gesture::AimGesture
