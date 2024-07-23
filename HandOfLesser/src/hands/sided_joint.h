#pragma once
#include <openxr/openxr.h>
#include <HandOfLesserCommon.h>

namespace HOL
{
	struct SidedJoint
	{
		XrHandJointEXT joint;
		HOL::HandSide side;
	};
}