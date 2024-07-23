#include "simple_gesture_detector.h"

namespace HOL::SimpleGesture
{
	SimpleGestureState getGesture(SimpleGestureType type, const OpenXRHand& hand)
	{
		switch (type)
		{
		case SimpleGestureType::IndexFingerPinch: {
			return {
				hand.aimState.pinchStrengthIndex,
				hand.aimState.pinchStrengthIndex >= 1.0f,
			};
		}

		case SimpleGestureType::MiddleFingerPinch: {
			return {
				hand.aimState.pinchStrengthMiddle,
				hand.aimState.pinchStrengthMiddle >= 1.0f,
			};
		}

		case SimpleGestureType::RingFingerPinch: {
			return {
				hand.aimState.pinchStrengthRing,
				hand.aimState.pinchStrengthRing >= 1.0f,
			};
		}

		case SimpleGestureType::PinkyFingerPinch: {
			return {
				hand.aimState.pinchStrengthLittle,
				hand.aimState.pinchStrengthLittle >= 1.0f,
			};
		}

		case SimpleGestureType::OpenHandFacingFace: {
			return {
				(hand.aimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB)
						== XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB
					? 1.0f
					: 0.0f,
				(hand.aimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB)
					== XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB,
			};
		}
		}
	}

	void populateGestures(SimpleGestureState* stateArray, const OpenXRHand& hand)
	{
		for (int i = 0; i < SimpleGestureType::SIMPLE_GESTURE_MAX; i++)
		{
			stateArray[i] = getGesture((SimpleGestureType)i, hand);
		}
	}
} // namespace HOL::SimpleGesture
