#include <string>
#include <cstring>
#include <string_view>
#include "input_wrapper.h"

namespace HOL::SteamVR
{
	namespace
	{
		bool endsWith(std::string_view value, std::string_view suffix)
		{
			if (value.size() < suffix.size())
			{
				return false;
			}

			return value.substr(value.size() - suffix.size()) == suffix;
		}
	}

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

	bool isTouchInputPath(const std::string& inputPath)
	{
		return endsWith(inputPath, "/touch");
	}

	bool isClickInputPath(const std::string& inputPath)
	{
		return endsWith(inputPath, "/click");
	}

	std::string getLogicalButtonPath(const std::string& inputPath)
	{
		if (isTouchInputPath(inputPath))
		{
			return inputPath.substr(0, inputPath.size() - std::strlen("/touch"));
		}

		if (isClickInputPath(inputPath))
		{
			return inputPath.substr(0, inputPath.size() - std::strlen("/click"));
		}

		return inputPath;
	}

	std::string formatLogicalButtonLabel(const std::string& buttonPath)
	{
		const std::string prefix = "/input/";
		if (buttonPath.rfind(prefix, 0) == 0)
		{
			return buttonPath.substr(prefix.size());
		}

		return buttonPath;
	}

} // namespace HOL::SteamVR
