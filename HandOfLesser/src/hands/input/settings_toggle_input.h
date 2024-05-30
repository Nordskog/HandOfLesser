#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/vrchat/vrchat_input.h"

namespace HOL
{
	// There aren't many settings you'd want to toggle via gestures,
	// so this will just live here for now.
	enum HolSetting
	{
		Default,
		SendOscInput,
		HolSetting_MAX,
	};

	class SettingsToggleInput : public BaseInput<bool>
	{
	public:
		static std::shared_ptr<SettingsToggleInput> Create()
		{
			return std::make_shared<SettingsToggleInput>();
		}

		void setup(HolSetting targetSetting);

		void submit(bool inputData) override;

	private:
		HolSetting mTargetSetting = HolSetting::Default;
		bool mLastValue = false;
	};
} // namespace HOL