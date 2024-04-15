#include "windows_utils.h"
#include <iostream>
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