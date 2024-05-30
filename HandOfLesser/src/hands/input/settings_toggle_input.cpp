#include "settings_toggle_input.h"
#include "src/core/settings_global.h"
#include <cstdio>

namespace HOL
{
	void SettingsToggleInput::submit(bool inputData)
	{
		// Expect to receive false/true on every frame, so deal with toggle ourselves
		if (!mLastValue && inputData)
		{
			printf("Button triggered!\n");

			// Changed from false to true, do the toggle.
			switch (this->mTargetSetting)
			{
				case HolSetting::Default: {
					break;
				}

				case HolSetting::SendOscInput: {
					Config.vrchat.sendOscInput = !Config.vrchat.sendOscInput;
					break;
				}
			}
		}

		mLastValue = inputData;
	}

	void SettingsToggleInput::setup(HolSetting targetSetting)
	{
		this->mTargetSetting = targetSetting;
	}

} // namespace HOL