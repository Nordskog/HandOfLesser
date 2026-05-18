#include "hold_gesture.h"

namespace HOL::Gesture::HoldGesture
{
	void Gesture::setGesture(const std::shared_ptr<BaseGesture::Gesture>& gesture)
	{
		this->mGesture = gesture;
		this->mSubGestures.clear();
		if (gesture)
		{
			this->mSubGestures.push_back(gesture);
		}
	}

	float Gesture::evaluateInternal(GestureData data)
	{
		if (!this->mGesture)
		{
			return 0.0f;
		}

		float value = this->mGesture->evaluate(data);
		const bool active = value >= 1.0f;

		if (!active)
		{
			this->mWasActive = false;
			return 0.0f;
		}

		if (!this->mWasActive)
		{
			this->mActivationStartTime = std::chrono::steady_clock::now();
			this->mWasActive = true;
		}

		if (std::chrono::steady_clock::now() - this->mActivationStartTime < this->parameters.duration)
		{
			return 0.0f;
		}

		return value;
	}
} // namespace HOL::Gesture::HoldGesture
