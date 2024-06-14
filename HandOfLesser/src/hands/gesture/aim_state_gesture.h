#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	class AimStateGesture : public BaseGesture
	{

	public:
		AimStateGesture() : BaseGesture(){};
		static std::shared_ptr<AimStateGesture> Create()
		{
			return std::make_shared<AimStateGesture>();
		}

		void setup(HOL::FingerType finger, HOL::HandSide side);

	private:
		HOL::FingerType mFinger;
		HOL::HandSide mSide;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
