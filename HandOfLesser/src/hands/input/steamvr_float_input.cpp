#include "steamvr_float_input.h"
#include "src/steamvr/steamvr_input.h"
#include <cstdio>

namespace HOL
{
	std::shared_ptr<SteamVRFloatInput> SteamVRFloatInput::setup(HOL::HandSide side,
																const std::string input)
	{
		mInput = input;
		mSide = side;
		return shared_from_this();
	}

	void SteamVRFloatInput::submit(float inputData)
	{
		// don't send if same
		if (inputData != this->mLastValue)
		{
			SteamVR::SteamVRInput::Current->submitFloat(mSide, mInput, inputData);
			this->mLastValue = inputData;
		}

	}
} // namespace HOL