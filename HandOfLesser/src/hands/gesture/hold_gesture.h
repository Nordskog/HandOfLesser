#pragma once

#include "base_gesture.h"
#include <chrono>

namespace HOL::Gesture::HoldGesture
{
	struct Parameters
	{
		std::chrono::milliseconds duration = std::chrono::milliseconds(2000);
	};

	class Gesture : public BaseGesture::Gesture
	{
	public:
		Gesture()
		{
			this->name = "HoldGesture";
		}

		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		void setGesture(const std::shared_ptr<BaseGesture::Gesture>& gesture);

		Parameters parameters;

	protected:
		float evaluateInternal(GestureData data) override;

	private:
		std::shared_ptr<BaseGesture::Gesture> mGesture;
		std::chrono::steady_clock::time_point mActivationStartTime{};
		bool mWasActive = false;
	};
} // namespace HOL::Gesture::HoldGesture
