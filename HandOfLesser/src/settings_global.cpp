#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

namespace HOL
{
	namespace settings
	{
		extern XrVector3f OrientationOffset(0, 0, 0);
		extern XrVector3f PositionOffset(0, 0, 0);

		extern XrVector3f FinalOrientationOffsetLeft(0, 0, 0);
		extern XrVector3f FinalPositionOffsetLeft(0, 0, 0);

		extern XrVector3f FinalOrientationOffsetRight(0, 0, 0);
		extern XrVector3f FinalPositionOffsetRight(0, 0, 0);
	}
}