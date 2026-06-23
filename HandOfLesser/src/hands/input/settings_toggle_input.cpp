#include "settings_toggle_input.h"
#include "src/core/settings_global.h"
#include <cstdio>

namespace HOL
{
	void SettingsToggleInput::submit(float inputData)
	{
		bool active = inputData >= 1.0f;

		// Expect to receive false/true on every frame, so deal with toggle ourselves
		if (active != mLastValue && active)
		{
			// Changed from false to true, do the toggle.
			switch (this->mTargetSetting)
			{
				case HolSetting::Default: {
					break;
				}

				case HolSetting::SendSteamVRInput: {
					Config.steamvr.sendSteamVRInput = !Config.steamvr.sendSteamVRInput;
					break;
				}
			}
		}

		mLastValue = active;
	}

	std::shared_ptr<SettingsToggleInput> SettingsToggleInput::setup(HolSetting targetSetting)
	{
		this->mTargetSetting = targetSetting;
		return shared_from_this();
	}

} // namespace HOL
