#pragma once

#include <HandOfLesserCommon.h>

namespace HOL
{
	class GenericControllerInterface
	{
	public:
		virtual void UpdatePose(HOL::HandTransformPacket* packet) = 0;
		virtual void UpdateInput(HOL::ControllerInputPacket* packet) = 0;
		virtual void UpdateSkeletal(HOL::SkeletalPacket* packet) = 0;
		virtual void UpdateBoolInput(const std::string& input, bool value) = 0;
		virtual void UpdateFloatInput(const std::string& input, float value) = 0;
		virtual void SubmitPose() = 0;
	};
} // namespace HOL
