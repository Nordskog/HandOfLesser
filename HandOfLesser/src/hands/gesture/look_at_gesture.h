#pragma once

#include "base_gesture.h"

namespace HOL::Gesture::LookAtGesture
{
	struct Parameters
	{
		HOL::HandSide side = HOL::LeftHand;
		float fovDegrees = 45.0f;
	};

	class Gesture : public BaseGesture::Gesture
	{
	public:
		Gesture()
		{
			this->name = "LookAtGesture";
		}

		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		Parameters parameters;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL::Gesture::LookAtGesture
