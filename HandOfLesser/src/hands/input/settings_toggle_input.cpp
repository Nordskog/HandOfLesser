#include "settings_toggle_input.h"
#include "src/core/settings_global.h"
#include <cstdio>

namespace HOL
{
	void SettingsToggleInput::submit(float inputData)
	{
		// Expect to receive false/true on every frame, so deal with toggle ourselves
		if (inputData != mLastValue && inputData >= 1)
		{
			// Changed from false to true, do the toggle.
			switch (this->mTargetSetting)
			{
				case HolSetting::Default: {
					break;
				}

				case HolSetting::SendOscInput: {
					Config.input.sendOscInput = !Config.input.sendOscInput;
					break;
				}

				case HolSetting::SendSteamVRInput: {
					Config.input.sendSteamVRInput = !Config.input.sendSteamVRInput;
					break;
				}
			}
		}

		mLastValue = inputData;
	}

	std::shared_ptr<SettingsToggleInput> SettingsToggleInput::setup(HolSetting targetSetting)
	{
		this->mTargetSetting = targetSetting;
		return shared_from_this();
	}

} // namespace HOL