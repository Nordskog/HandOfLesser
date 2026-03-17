#pragma once

#include "base_gesture.h"

namespace HOL::Gesture::InverseGesture
{
	class Gesture : public BaseGesture::Gesture
	{
	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "InverseGesture";
		};

		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		void setGesture(std::shared_ptr<BaseGesture::Gesture> gesture);

	protected:
		float evaluateInternal(GestureData data) override;

	private:
		std::shared_ptr<BaseGesture::Gesture> mGesture;
	};
} // namespace HOL::Gesture::InverseGesture
