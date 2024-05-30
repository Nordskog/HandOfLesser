#pragma once

#include "base_gesture.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	class AboveBelowCurlPlaneGesture : public BaseGesture
	{

	public:
		AboveBelowCurlPlaneGesture() : BaseGesture(){};
		static std::shared_ptr<AboveBelowCurlPlaneGesture> Create()
		{
			return std::make_shared<AboveBelowCurlPlaneGesture>();
		}

		void setup(HOL::FingerType planeFinger, HOL::FingerType otherFinger, HOL::HandSide side);

	private:
		HOL::FingerType mPlaneFinger;
		HOL::FingerType mOtherFinger;
		HOL::HandSide mSide;

	protected:
		float evaluateInternal(GestureData data) override;
	};
} // namespace HOL
