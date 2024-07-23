#include "osc_float_input.h"
#include <cstdio>

namespace HOL
{
	std::shared_ptr<OscFloatInput> OscFloatInput::setup(std::string input)
	{
		mInput = input;
		return shared_from_this();
	}

	void OscFloatInput::submit(float inputData)
	{
		VRChat::VRChatInput::Current->submitFloat(this->mInput, inputData);
	}
} // namespace HOL