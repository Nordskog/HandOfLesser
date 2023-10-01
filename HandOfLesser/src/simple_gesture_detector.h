#pragma once

#include "TrackedHand.h"
#include "simple_gesture.h"

namespace HOL
{
	namespace SimpleGesture
	{
		SimpleGestureState getGesture(SimpleGestureType type, TrackedHand* hand);

		void populateGestures(SimpleGestureState* stateArray, TrackedHand* hand);
	}
}

