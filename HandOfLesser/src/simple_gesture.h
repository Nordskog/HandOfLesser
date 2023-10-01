#pragma once

#include "TrackedHand.h"

namespace HOL
{
	namespace SimpleGesture
	{
		enum SimpleGestureType
		{
			IndexFingerPinch,
			MiddleFingerPinch,
			RingFingerPinch,
			PinkyFingerPinch,
			IndexMiddlePinkyGrip,
			OpenHandFacingFace,

			SIMPLE_GESTURE_MAX

		};

		struct SimpleGestureState
		{
			float value = 0;
			bool click = false;
		};
	}
}

