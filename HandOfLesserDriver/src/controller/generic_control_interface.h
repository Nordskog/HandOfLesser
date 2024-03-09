#pragma once

#include <HandOfLesserCommon.h>

namespace HOL
{
	 class GenericControllerInterface
	{
	public:
		virtual void UpdatePose(HOL::HandTransformPacket* packet) = 0;
		virtual void UpdateInput(HOL::ControllerInputPacket* packet) = 0;
		virtual void SubmitPose() = 0;
	};
}

