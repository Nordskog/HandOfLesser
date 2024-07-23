#include "osc_alternate_float_input.h"
#include <cstdio>

namespace HOL
{
	void OscAlternateFloatInput::setup(std::string inputOn, std::string inputOff)
	{
		mInputOn = inputOn;
		mInputOff = inputOff;
	}

	void OscAlternateFloatInput::submit(float inputData)
	{
		// If 1 send 1 to ON and 0 to OFF
		// if less than 1, send 0 to ON and 1 to OFF

		if (inputData < 1)
			inputData = 0;

		VRChat::VRChatInput::Current->submitFloat(this->mInputOn, inputData);
		VRChat::VRChatInput::Current->submitFloat(this->mInputOff, 1.f - inputData);
	}
} // namespace HOL