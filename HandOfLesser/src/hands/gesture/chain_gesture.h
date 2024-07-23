#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>
#include <chrono>

using namespace std::chrono_literals;

namespace HOL::Gesture::ChainGesture
{
	struct Parameters
	{
		std::chrono::milliseconds maxDelay;
	};

	class Gesture : public BaseGesture::Gesture
	{

	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "ChainGesture";
		};
		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		void addGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

		ChainGesture::Parameters parameters;

	private:
		std::vector<std::shared_ptr<BaseGesture::Gesture>> mChainedGestures;
		std::chrono::milliseconds mMaxDelay = 500ms;
		std::chrono::steady_clock::time_point mLastActivation;
		int mCurrentGestureIndex = 0; // Iterate as each gesture is activated
		bool mActivated = false;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
