#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

namespace HOL
{
	namespace settings
	{
		extern XrVector3f OrientationOffset;
		extern XrVector3f PositionOffset;

		extern XrVector3f FinalOrientationOffsetLeft;
		extern XrVector3f FinalPositionOffsetLeft;

		extern XrVector3f FinalOrientationOffsetRight;
		extern XrVector3f FinalPositionOffsetRight;
	}
}