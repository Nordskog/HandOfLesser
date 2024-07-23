#include "base_gesture.h"

namespace HOL::Gesture
{
	float BaseGesture::Gesture::evaluate(GestureData data)
	{
		return this->lastValue = this->evaluateInternal(data);
	}

	std::vector<std::shared_ptr<BaseGesture::Gesture>>& BaseGesture::Gesture::getSubGestures()
	{
		return this->mSubGestures;
	}
} // namespace HOL::Gesture

