#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL::Gesture::FingerCurlGesture
{
	struct Paremters
	{
		HOL::FingerType finger;
		HOL::HandSide side;
		bool first = true;
		bool second = true;
		bool third = false;
		float minDegrees = 0;
		float maxDegrees = 74;	// Per joint
	};

	class Gesture : public BaseGesture::Gesture
	{

	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "FingerCurlGesture";
		};
		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		FingerCurlGesture::Paremters parameters;

	private:


	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
