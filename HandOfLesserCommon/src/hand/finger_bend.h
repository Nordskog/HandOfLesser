#pragma once

#include "hand.h"

namespace HOL
{
	struct FingerBend
	{
		float bend[FingerBendType::FingerBendType_MAX];
		float getCurlSum();
		float getCurlSumWithoutDistal();
		void setSplay(float humanoidSplay);
	};
} // namespace HOL
