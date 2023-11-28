#include "simple_gesture_detector.h"

namespace HOL::SimpleGesture
{
	SimpleGestureState getGesture(SimpleGestureType type, OpenXRHand* hand)
	{
		switch (type)
		{
		case SimpleGestureType::IndexFingerPinch: {
			return {
				hand->mAimState.pinchStrengthIndex,
				hand->mAimState.pinchStrengthIndex >= 1.0f,
			};
		}

		case SimpleGestureType::MiddleFingerPinch: {
			return {
				hand->mAimState.pinchStrengthMiddle,
				hand->mAimState.pinchStrengthMiddle >= 1.0f,
			};
		}

		case SimpleGestureType::RingFingerPinch: {
			return {
				hand->mAimState.pinchStrengthRing,
				hand->mAimState.pinchStrengthRing >= 1.0f,
			};
		}

		case SimpleGestureType::PinkyFingerPinch: {
			return {
				hand->mAimState.pinchStrengthLittle,
				hand->mAimState.pinchStrengthLittle >= 1.0f,
			};
		}

		case SimpleGestureType::OpenHandFacingFace: {
			return {
				(hand->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB)
						== XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB
					? 1.0f
					: 0.0f,
				(hand->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB)
					== XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB,
			};
		}
		}
	}

	void populateGestures(SimpleGestureState* stateArray, OpenXRHand* hand)
	{
		for (int i = 0; i < SimpleGestureType::SIMPLE_GESTURE_MAX; i++)
		{
			stateArray[i] = getGesture((SimpleGestureType)i, hand);
		}
	}
} // namespace HOL::SimpleGesture
