#pragma once

#include "openxr_hand.h"

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
	} // namespace SimpleGesture
} // namespace HOL
