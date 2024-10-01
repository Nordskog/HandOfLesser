#pragma once

#include "settings_global.h"
#include <HandOfLesserCommon.h>

namespace HOL
{
	settings::HandOfLesserSettings Config;

	namespace settings
	{
		void restoreDefaultControllerOffset(HOL::ControllerOffsetPreset type)
		{
			PoseLocationEuler def = HOL::getControllerOffsetPreset(type);

			Config.handPose.positionOffset = def.position;
			Config.handPose.orientationOffset = def.orientation;
		}

	} // namespace settings
} // namespace HOL
