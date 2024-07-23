#pragma once

#include "src/hands/input/base_input.h"
#include <string>
#include "steamvr_input.h"

namespace HOL::SteamVR
{
	SteamVRInput* SteamVRInput::Current = nullptr;

	SteamVRInput::SteamVRInput()
	{
		SteamVRInput::Current = this;
	}

	void SteamVRInput::submitFloat(HandSide side, const std::string& inputName, float value)
	{
		//printf("SteamVR Input: %s, %.3f\n", inputName.c_str(), value);

		FloatInputPacket input;
		input.side = side;
		std::strncpy(&input.inputName[0], inputName.c_str(), 64); // max length 64
		input.value = value;

		this->floatInputs.push_back(input);
	}

	void SteamVRInput::submitBoolean(HandSide side, const std::string& inputName, bool value)
	{
		printf("SteamVR Input: %s, %s\n", inputName.c_str(), value ? "True" : "False");

		BoolInputPacket input;
		input.side = side;
		std::strncpy(&input.inputName[0], inputName.c_str(), 64); // max length 64
		input.value = value;

		this->boolInputs.push_back(input);
	}

	void SteamVRInput::clear()
	{
		this->boolInputs.clear();
		this->floatInputs.clear();
	}


} // namespace HOL::VRChat