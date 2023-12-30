#pragma once

#include <string>
#include <Windows.h>

bool RegGetString(HKEY hKey, const std::string& subKey, const std::string& value, std::string& out);
