#pragma once

#include "base_action.h"

namespace HOL
{
	BaseAction::BaseAction(std::initializer_list<InputType> supportedInputs)
	{
		mSupportedInputs = supportedInputs;
	}

	void BaseAction::evaluate(GestureData data)
	{
		float triggerGesture = this->mTriggerGesture->evaluate(data);
		float holdGesture = triggerGesture;

		// gestureValue should be set regardless of down/up states
		// maybe have separate gesture for this value?
		this->mActionData.gestureValue = triggerGesture;

		if (this->mUseHoldGesture)
		{
			holdGesture = this->mTriggerGesture->evaluate(data);
		}

		// Regardless of whether a separate hold gesture is present, 
		// the releaseThreshold should be respected.
		if (holdGesture >= this->mParameters.releaseThreshold)
		{
			// If above threshold, treat as 1.
			holdGesture = 1;
		}

		// Down if trigger down or already down and hold down.
		if (triggerGesture >= 1 || (holdGesture >= 1 && this->mActionData.isDown))
		{
			// Can't be up when you're down
			mDelayedUp = false;

			// If not already down, update down time
			if (!mDelayedDown && !this->mActionData.isDown)
			{
				// We count taps from the first down
				if (this->timeSinceDown() < this->mParameters.maxTapTime
					&& this->timeSinceDown() > this->mParameters.minTapTime)
				{
					this->mTapCount++;
				}
				else
				{
					this->mTapCount = 1;
				}

				mDelayedDown = true;
				this->mDownTime = std::chrono::steady_clock::now();
			}

			// Block state update until min time
			if (this->timeSinceDown() > this->mParameters.minDownTime
				&& mTapCount >= this->mParameters.minTapCount)
			{
				mDelayedDown = false;
				// On down only true on first frame of down
				this->mActionData.onDown = !this->mActionData.isDown;
				this->mActionData.isDown = true;
				this->mActionData.onUp = false;
			}
		}
		else
		{
			if (this->mActionData.isDown || mDelayedDown)
			{
				// If not already down, update down time
				if (!mDelayedUp)
				{
					mDelayedUp = this->mActionData.isDown; // Only if fully down
					this->mUpTime = std::chrono::steady_clock::now();
				}

				// Block state update until min time
				if (this->timeSinceUp() > this->mParameters.minReleaseTime)
				{
					mDelayedUp = false;

					// onUp for one frame after exit
					// But only we we were fully down
					this->mActionData.onUp = this->mActionData.isDown;

					// On up after being fully down, reset tap counter
					if (this->mActionData.isDown)
					{
						this->mTapCount = 0;
					}

					this->mActionData.onDown = false;
					this->mActionData.isDown = false;
				}
			}
			else
			{
				mDelayedUp = false;
				this->mActionData.onUp = false;
				this->mActionData.onDown = false;
				this->mActionData.isDown = false;
			}

			// Can't be down when we're up
			// But do late so we can still tell if we were down
			mDelayedDown = false;
		}

		// Touch can just be simple I guess
		// What is actually
		mActionData.isTouch
			= triggerGesture > this->mParameters.touchThreshold || mActionData.isDown;

		this->onEvaluate(data, this->mActionData);
	}

	void BaseAction::setTriggerGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mTriggerGesture = gesture;
	}

	void BaseAction::setTapGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mTapGesture = gesture;
	}

	void BaseAction::setHoldGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mHoldGesture = gesture;
		this->mUseHoldGesture = false;
	}

	void BaseAction::setParameters(ActionParameters params)
	{
		this->mParameters = params;
	}

	// Editable!
	ActionParameters& BaseAction::getParameters()
	{
		return this->mParameters;
	}

	std::chrono::milliseconds BaseAction::timeSinceDown()
	{
		auto currentTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mDownTime);
	}

	std::chrono::milliseconds BaseAction::timeSinceUp()
	{
		auto currentTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mUpTime);
	}

	void BaseAction::submitInput(InputType type, float value)
	{
		for (auto& input : mInputs[type])
		{
			input->submit(value);
		}
	}

	void BaseAction::addSink(InputType type, std::shared_ptr<BaseInput<float>> input )
	{
		this->mInputs[type].push_back(input);
	}
} // namespace HOL