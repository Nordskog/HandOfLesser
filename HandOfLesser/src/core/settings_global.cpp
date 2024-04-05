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

			Config.handPose.PositionOffset = def.position;
			Config.handPose.OrientationOffset = def.orientation;
		}

	} // namespace settings
} // namespace HOL
