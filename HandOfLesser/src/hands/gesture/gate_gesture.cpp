#include "gate_gesture.h"

#include <algorithm>

namespace HOL::Gesture::GateGesture
{
	void Gesture::setTriggerGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mTriggerGesture = gesture;
		this->mSubGestures.clear();
		if (this->mTriggerGesture)
		{
			this->mSubGestures.push_back(this->mTriggerGesture);
		}
		if (this->mModifierGesture)
		{
			this->mSubGestures.push_back(this->mModifierGesture);
		}
	}

	void Gesture::setModifierGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mModifierGesture = gesture;
		this->mSubGestures.clear();
		if (this->mTriggerGesture)
		{
			this->mSubGestures.push_back(this->mTriggerGesture);
		}
		if (this->mModifierGesture)
		{
			this->mSubGestures.push_back(this->mModifierGesture);
		}
	}

	float Gesture::evaluateInternal(GestureData data)
	{
		if (!this->mTriggerGesture)
		{
			return 0.0f;
		}

		float triggerValue = this->mTriggerGesture->evaluate(data);
		if (!this->mModifierGesture)
		{
			return triggerValue;
		}

		float modifierValue = std::clamp(this->mModifierGesture->evaluate(data), 0.0f, 1.0f);
		auto now = std::chrono::steady_clock::now();
		bool triggerActive = triggerValue >= 1.0f;
		bool modifierActive = modifierValue >= 1.0f;

		if (modifierActive)
		{
			if (!this->mModifierWasActive)
			{
				this->mModifierActiveSince = now;
				this->mModifierWasActive = true;
			}
		}
		else
		{
			this->mModifierWasActive = false;
		}

		bool modifierPrimed
			= modifierActive
			  && (now - this->mModifierActiveSince) >= this->parameters.requiredLeadTime;

		if (!modifierPrimed && triggerActive)
		{
			this->mBlockedUntilReleased = true;
		}

		if (this->mBlockedUntilReleased)
		{
			if (triggerValue <= 0.0f)
			{
				this->mBlockedUntilReleased = false;
			}
			else
			{
				return 0.0f;
			}
		}

		if (!modifierPrimed)
		{
			return 0.0f;
		}

		return triggerValue;
	}
} // namespace HOL::Gesture::GateGesture
