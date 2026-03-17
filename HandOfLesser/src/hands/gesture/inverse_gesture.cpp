#include "inverse_gesture.h"
#include <algorithm>

namespace HOL::Gesture::InverseGesture
{
	void Gesture::setGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mGesture = gesture;
	}

	float Gesture::evaluateInternal(GestureData data)
	{
		if (!this->mGesture)
		{
			return 0.0f;
		}

		return 1.0f - std::clamp(this->mGesture->evaluate(data), 0.0f, 1.0f);
	}
} // namespace HOL::Gesture::InverseGesture
