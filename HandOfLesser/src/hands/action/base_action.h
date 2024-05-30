#pragma once

#include "src/hands/gesture/base_gesture.h"
#include "src/hands/input/base_input.h"
#include <chrono>

namespace HOL
{
	struct ActionData
	{
		float gestureValue = 0;
		bool onDown = false;
		bool isDown = false;
		bool onUp = false;
	};

	struct ActionParameters
	{
		std::chrono::milliseconds minDownTime;	  // Configurable long-press basically
		std::chrono::milliseconds minReleaseTime; // no up until this amount of time
		std::chrono::milliseconds maxTapTime;
		std::chrono::milliseconds minTapTime;

		// Iterate if down occurs within mMaxDoubleTapTime of up
		int minTapCount = 1; // taps required to trigger. 1 is single press.
		int tapCount = 0;
	};

	// BaseAction extends from this, because we cannot store generics in vectors otherwise.
	// ProtoAction can always be
	class ProtoAction
	{

	public:
		ProtoAction(){};

		void evaluate(GestureData data)
		{
			float triggerGesture = this->mTriggerGesture->evaluate(data);
			float holdGesture = triggerGesture;
			if (this->mUseHoldGesture)
			{
				holdGesture = this->mTriggerGesture->evaluate(data);
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

			this->onEvaluate(data, this->mActionData);
		}

		void setTriggerGesture(std::shared_ptr<BaseGesture> gesture)
		{
			this->mTriggerGesture = gesture;
		}

		void setTapGesture(std::shared_ptr<BaseGesture> gesture)
		{
			this->mTapGesture = gesture;
		}

		void setHoldGesture(std::shared_ptr<BaseGesture> gesture)
		{
			this->mHoldGesture = gesture;
			this->mUseHoldGesture = false;
		}

		void setParameters(ActionParameters params)
		{
			this->mParameters = params;
		}

		// Editable!
		ActionParameters& getParameters()
		{
			return this->mParameters;
		}

		std::chrono::milliseconds timeSinceDown()
		{
			auto currentTime = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mDownTime);
		}

		std::chrono::milliseconds timeSinceUp()
		{
			auto currentTime = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mUpTime);
		}

	private:
		// Must be 1 to enter action, equivalent to a button press
		std::shared_ptr<BaseGesture> mTriggerGesture;

		// Once activated, a separate gesture my be used to hold the action
		// if mUseHoldGesture the state of mTriggerGesture is used instead
		std::shared_ptr<BaseGesture> mHoldGesture;

		// Tap will not be increment unless this is <1 since previous tap
		// e.g. fingers must be moved apart fully for a double tap to register
		std::shared_ptr<BaseGesture> mTapGesture;

		// in MS
		std::chrono::steady_clock::time_point mDownTime;
		std::chrono::steady_clock::time_point mUpTime;
		bool mUseHoldGesture = false;

		// Keep track of when down/up time was set,
		// before we update the actual state
		bool mDelayedDown = false;
		bool mDelayedUp = false;

		int mTapCount = 0;

		ActionData mActionData;

		ActionParameters mParameters;

	protected:
		virtual void onEvaluate(GestureData gestureData, ActionData actionData)
		{
			return;
		}
	};

	// ProtoAction exists so we can store actions without specifying the generic component
	template <typename T> class BaseAction : public ProtoAction
	{
	protected:
		std::shared_ptr<BaseInput<T>> mInputSink;

	public:
		void setSink(std::shared_ptr<BaseInput<T>> sink)
		{
			this->mInputSink = sink;
		}
	};

} // namespace HOL