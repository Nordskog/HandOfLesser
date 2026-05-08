#include "inverse_gesture.h"
#include <algorithm>

namespace HOL::Gesture::InverseGesture
{
	void Gesture::setGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mGesture = gesture;
	}

	void Gesture::setBinaryThreshold(float threshold)
	{
		this->mUseBinaryThreshold = true;
		this->mBinaryThreshold = threshold;
	}

	float Gesture::evaluateInternal(GestureData data)
	{
		if (!this->mGesture)
		{
			return 0.0f;
		}

		float value = std::clamp(this->mGesture->evaluate(data), 0.0f, 1.0f);
		if (this->mUseBinaryThreshold)
		{
			return value < this->mBinaryThreshold ? 1.0f : 0.0f;
		}

		return 1.0f - value;
	}
} // namespace HOL::Gesture::InverseGesture
