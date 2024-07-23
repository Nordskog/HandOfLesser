#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>
#include <chrono>

using namespace std::chrono_literals;

namespace HOL::Gesture::ComboGesture
{
	struct Parameters
	{
		bool holdUntilAllReleased = true;
	};

	class Gesture : public BaseGesture::Gesture
	{

	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "ComboGesture";
		};
		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		void addGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

		ComboGesture::Parameters parameters;

	private:
		bool mActivated = false;
		std::vector<std::shared_ptr<BaseGesture::Gesture>> mComboGestures;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
