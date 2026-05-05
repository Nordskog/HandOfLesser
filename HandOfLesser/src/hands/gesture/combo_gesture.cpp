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
		float min = 1;
		float max = 0;
		for (auto& gesture : this->mComboGestures)
		{
			float val = gesture.get()->evaluate(data);
			if (val < min)
				min = val;
			if (val > max)
				max = val;
		}

		if (min >= 1)
		{
			this->mActivated = true;
			return 1;
		}

		if (this->parameters.holdUntilAllReleased && this->mActivated && max >= 1)
		{
			return 1;
		}

		this->mActivated = false;

		return max;
	}

	void ComboGesture::Gesture::addGesture(std::shared_ptr<BaseGesture::Gesture> gesture)
	{
		this->mComboGestures.push_back(gesture);
	}
}

