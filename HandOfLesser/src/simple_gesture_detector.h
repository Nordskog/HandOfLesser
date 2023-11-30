#pragma once

#include "openxr_hand.h"
#include "simple_gesture.h"

namespace HOL::SimpleGesture
{
	SimpleGestureState getGesture(SimpleGestureType type, OpenXRHand* hand);

	void populateGestures(SimpleGestureState* stateArray, OpenXRHand* hand);

} // namespace HOL::SimpleGesture
