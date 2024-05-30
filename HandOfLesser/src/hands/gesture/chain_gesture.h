#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>
#include <chrono>

using namespace std::chrono_literals;

namespace HOL
{
	class ChainGesture : public BaseGesture
	{

	public:
		ChainGesture() : BaseGesture(){};
		static std::shared_ptr<ChainGesture> Create()
		{
			return std::make_shared<ChainGesture>();
		}

		void setup(std::chrono::milliseconds maxDelay);

		void addGesture(std::shared_ptr<BaseGesture> gesture);

	private:
		std::vector<std::shared_ptr<BaseGesture>> mChainedGestures;
		std::chrono::milliseconds mMaxDelay = 500ms;
		std::chrono::steady_clock::time_point mLastActivation;
		int mCurrentGestureIndex = 0; // Iterate as each gesture is activated
		bool mActivated = false;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
