#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>




namespace HOL
{
	namespace settings
	{
		void restoreDefaultControllerOffset(ControllerOffsetPreset type);
	}

	extern HOL::settings::HandOfLesserSettings Config;

} // namespace HOL::settings