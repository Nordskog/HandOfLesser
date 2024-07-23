#pragma once

#include "src/hands/gesture/base_gesture.h"
#include "src/hands/input/base_input.h"
#include <chrono>

using namespace HOL::Gesture;

namespace HOL
{
	struct ActionData
	{
		float gestureValue = 0;
		bool onDown = false;
		bool isDown = false;
		bool onUp = false;
		bool isTouch = false;
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
		float touchThreshold = 0.5;
		float releaseThreshold = 1;
	};

	class BaseAction
	{

	public:
		BaseAction(std::initializer_list<InputType> inputs);

		void evaluate(GestureData data);

		void setTriggerGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

		void setTapGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

		void setHoldGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

		void setParameters(ActionParameters params);

		void addSink(InputType type, std::shared_ptr<BaseInput<float>> input);

		// Editable!
		ActionParameters& getParameters();

		std::chrono::milliseconds timeSinceDown();

		std::chrono::milliseconds timeSinceUp();

	private:
		// Must be 1 to enter action, equivalent to a button press
		std::shared_ptr<BaseGesture::Gesture> mTriggerGesture;

		// Once activated, a separate gesture my be used to hold the action
		// if mUseHoldGesture the state of mTriggerGesture is used instead
		std::shared_ptr<BaseGesture::Gesture> mHoldGesture;

		// Tap will not be increment unless this is <1 since previous tap
		// e.g. fingers must be moved apart fully for a double tap to register
		std::shared_ptr<BaseGesture::Gesture> mTapGesture;

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

		std::vector<std::shared_ptr<BaseInput<float>>> mInputs[InputType::InputType_MAX];

	protected:
		// These should be populated by the action
		// Name is used to determine the number of slots, and what values they receive.
		std::vector<InputType> mSupportedInputs;

		void submitInput(InputType type, float value);

		virtual void onEvaluate(GestureData gestureData, ActionData actionData)
		{
			return;
		}
	};

} // namespace HOL