#include "windows_utils.h"
#include <iostream>
#include <sstream>
#include <atlbase.h>

bool RegGetString(HKEY hKey, const std::string& subKey, const std::string& value, std::string& out)
{
	CRegKey regKey;
	LONG result = regKey.Open(HKEY_LOCAL_MACHINE, subKey.c_str(), KEY_READ);

	if (result == ERROR_SUCCESS)
	{
		char buffer[256];
		ULONG bufferSize = sizeof(buffer);
		result = regKey.QueryStringValue(value.c_str(), buffer, &bufferSize);

		if (result == ERROR_SUCCESS)
		{
			out = std::string(buffer);
			return true;
		}
		else
		{
			std::cerr << "Error reading registry value, error: " << result << std::endl;
		}

		regKey.Close();
	}
	else
	{
		std::cerr << "Error opening registry key, error: " << result << std::endl;
	}

	return false;
}

std::string GetWindowsErrorMessage(DWORD errorCode)
{
	char* messageBuffer = nullptr;
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
				  | FORMAT_MESSAGE_IGNORE_INSERTS;
	DWORD languageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	DWORD messageLength = FormatMessageA(flags,
										 nullptr,
										 errorCode,
										 languageId,
										 reinterpret_cast<LPSTR>(&messageBuffer),
										 0,
										 nullptr);

	if (messageLength == 0 || messageBuffer == nullptr)
	{
		return {};
	}

	std::string message(messageBuffer, messageLength);
	LocalFree(messageBuffer);

	while (!message.empty() && (message.back() == '\r' || message.back() == '\n'))
	{
		message.pop_back();
	}

	return message;
}

std::string FormatWindowsError(DWORD errorCode)
{
	std::ostringstream message;
	message << "0x" << std::hex << std::uppercase << errorCode;

	std::string windowsMessage = GetWindowsErrorMessage(errorCode);
	if (!windowsMessage.empty())
	{
		message << " (" << windowsMessage << ")";
	}

	return message.str();
}
