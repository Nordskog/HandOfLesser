#pragma once

#include <string>
#include <HandOfLesserCommon.h>
#include <vector>

namespace HOL::SteamVR
{
	class SteamVRInput
	{
	public:
		SteamVRInput();
		static SteamVRInput* Current;
		void submitFloat(HandSide side, const std::string& inputName, float value);
		void submitBoolean(HandSide side, const std::string& inputName, bool value);
		void clear();

		std::vector<HOL::FloatInputPayload> floatInputs;
		std::vector<HOL::BoolInputPayload> boolInputs;


	};

} // namespace HOL::VRChat
