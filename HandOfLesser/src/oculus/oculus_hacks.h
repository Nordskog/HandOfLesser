#pragma once

#include <windows.h> // Because HANDLE

namespace HOL::hacks
{

	void fixOvrSessionStateRestriction();
	HMODULE getModule(HANDLE hProcess, const TCHAR moduleName[]);
} // namespace HOL::hacks
