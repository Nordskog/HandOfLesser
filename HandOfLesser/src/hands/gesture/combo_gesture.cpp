#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/openxr/xr_hand_utils.h"
#include "src/core/ui/user_interface.h"
#include "chain_gesture.h"
#include "src/util/hol_utils.h"
#include "combo_gesture.h"

using namespace HOL;
using namespace HOL::OpenXR;

namespace HOL::Gesture
{
	float ComboGesture::Gesture::evaluateInternal(GestureData data)
	{
		// float average = 0;
		float min = 1;
		for (auto& gesture : this->mComboGestures)
		{
			float val = gesture.get()->evaluate(data);
			if (val < min)
				min = val;
			// average += val;
		}
		// average /= (float)this->mComboGestures.size();

		if (min >= 1)
		{
			this->mActivated = true;
			return 1;
		}

		if (mActivated && this->parameters.holdUntilAllReleased && min >= 1.f)
		{
			return true;
		}
		else
		{
			this->mActivated = false;
		}

		return min;
	}

	void ComboGesture::Gesture::addGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mComboGestures.push_back(gesture);
	}
}

