#include "steamvr_bool_input.h"
#include "src/steamvr/steamvr_input.h"
#include <cstdio>

namespace HOL
{
	std::shared_ptr<SteamVRBoolInput> SteamVRBoolInput::setup(HOL::HandSide side,
															  const std::string input)
	{
		mInput = input;
		mSide = side;
		return shared_from_this();
	}

	void SteamVRBoolInput::submit(float inputData)
	{
		bool newValue = inputData >= 1.f;
		// don't send if same
		if (newValue != this->mLastValue)
		{
			SteamVR::SteamVRInput::Current->submitBoolean(mSide, mInput, newValue);
			this->mLastValue = newValue;
		}

	}
} // namespace HOL