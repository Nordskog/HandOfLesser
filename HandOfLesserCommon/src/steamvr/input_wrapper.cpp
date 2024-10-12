#pragma once

#include <string>
#include "input_wrapper.h"

namespace HOL::SteamVR
{

	// e.g. input/x then call touch() to get input/x/touch
	SteamVR::InputWrapper::InputWrapper(std::string input, HandleType type)
	{
		switch (type)
		{
			case HandleType::input:
				this->mBaseInput = "/input/" + input;
				break;
			case HandleType::output:
				this->mBaseInput = "/output/" + input;
				break;
			case HandleType::skeleton:
				this->mBaseInput = "/skeleton/" + input;
				break;
			case HandleType::pose:
				this->mBaseInput = "/pose/" + input;
				break;
		}
	}

	std::string InputWrapper::touch() const
	{
		return this->mBaseInput + "/" + "touch";
	}

	std::string InputWrapper::click() const
	{
		return this->mBaseInput + "/" + "click";
	}

	std::string InputWrapper::value() const
	{
		return this->mBaseInput + "/" + "value";
	}

	std::string InputWrapper::x() const
	{
		return this->mBaseInput + "/" + "x";
	}

	std::string InputWrapper::y() const
	{
		return this->mBaseInput + "/" + "y";
	}

	std::string InputWrapper::force() const
	{
		return this->mBaseInput + "/" + "force";
	}

	std::string InputWrapper::index() const
	{
		return this->mBaseInput + "/" + "index";
	}

	std::string InputWrapper::middle() const
	{
		return this->mBaseInput + "/" + "middle";
	}

	std::string InputWrapper::ring() const
	{
		return this->mBaseInput + "/" + "ring";
	}

	std::string InputWrapper::pinky() const
	{
		return this->mBaseInput + "/" + "pinky";
	}




} // namespace HOL::SteamVR