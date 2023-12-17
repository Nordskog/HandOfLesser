#pragma once

#include "simple_gesture.h"
#include <src/openxr/openxr_hand.h>

namespace HOL::SimpleGesture
{
	SimpleGestureState getGesture(SimpleGestureType type, OpenXRHand* hand);

	void populateGestures(SimpleGestureState* stateArray, OpenXRHand* hand);

} // namespace HOL::SimpleGesture
