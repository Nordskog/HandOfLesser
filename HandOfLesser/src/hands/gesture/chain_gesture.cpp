#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"
#include "src/core/ui/user_interface.h"
#include "chain_gesture.h"
#include "src/util/hol_utils.h"

using namespace HOL::OpenXR;

namespace HOL::Gesture
{
	float ChainGesture::Gesture::evaluateInternal(GestureData data)
	{
		// printf("Evaluate\n");

		if (this->mCurrentGestureIndex > this->mChainedGestures.size())
		{
			printf("index Out of range\n");
			return 0;
		}

		auto currGesture = this->mChainedGestures[this->mCurrentGestureIndex];
		float curreGestureValue = currGesture.get()->evaluate(data);

		// Not on first gesture, and final gesture has not been activated
		if (!mActivated && mCurrentGestureIndex > 0
			&& timeSince(this->mLastActivation) > this->mMaxDelay)
		{
			// Time threshold exceeded, reset.
			this->mCurrentGestureIndex = 0;
			this->mActivated = false;
			return 0;
		}

		// printf("Current Value: %.3f", curreGestureValue);

		// // We're on the final gesture
		if (this->mCurrentGestureIndex == this->mChainedGestures.size() - 1)
		{
			if (curreGestureValue >= 1)
			{
				// Gesture is active
				this->mActivated = true;
				return curreGestureValue;
			}
			else
			{
				if (this->mActivated)
				{
					// Gesture was active, so we reset
					this->mCurrentGestureIndex = 0;
					this->mActivated = false;
					return 0;
				}
			}
		}
		else
		{
			// Any gesture but the final one, and we have not exceeded
			// time threshold. If active, iterate counter and set time.
			if (curreGestureValue >= 1.f)
			{

				this->mCurrentGestureIndex++;
				this->mLastActivation = std::chrono::steady_clock::now();
				return 0;
			}
		}
	}

	void ChainGesture::Gesture::addGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mChainedGestures.push_back(gesture);
	}

}

