#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL::Gesture::AimGesture
{
	struct Parameters
	{
		HOL::HandSide side = HOL::HandSide::LeftHand;
		XrHandTrackingAimFlagsFB flags = 0;
	};

	class Gesture : public BaseGesture::Gesture
	{
	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "AimGesture";
		};

		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		AimGesture::Parameters parameters;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL::Gesture::AimGesture
