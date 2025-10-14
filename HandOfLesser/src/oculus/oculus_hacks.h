#pragma once

#include <windows.h> // Because HANDLE

namespace HOL::hacks
{
	void applyPatch(HMODULE hModule, const BYTE* targetBytes,
					size_t targetByteSize,
					const BYTE* targetInstruction,
					size_t targetInstructionSize,
					const BYTE* replacementInstruction,
					size_t replacementInstructionSize
		);

	void fixOvrSessionStateRestriction();
	void fixOvrMultimodalSupportCheck();
	HMODULE getModule(HANDLE hProcess, const TCHAR moduleName[]);
} // namespace HOL::hacks
