#include "oculus_hacks.h"

#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <iomanip>

namespace HOL::hacks
{
	HMODULE getModule(HANDLE hProcess, const TCHAR moduleName[])
	{
		// module meaning dll
		// all these weird functions use tchar, so I guess we'll use it too.

		HMODULE hMods[1024];
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				TCHAR szModName[MAX_PATH];

				if (GetModuleFileNameEx(
						hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)
					))
				{
					if (_tcsstr(szModName, moduleName) != NULL)
					{
						std::cout << "Found OVR dll: " << szModName << std::endl;
						return hMods[i];
					}
				}
			}
		}

		std::cerr << "Could not find OVR dll!" << std::endl;

		return nullptr;
	}

	/// <summary>
	/// Modifies ovr_GetHandPose LibOVRRT64_1.dll and inverts the clause that
	/// presumably checks if the session state is FOCUSED by replacing a jnz instruction with jz
	/// </summary>
	void fixOvrSessionStateRestriction()
	{
		HANDLE currentProcess = GetCurrentProcess();
		HMODULE hModule = getModule(
			currentProcess, _T("\LibOVRRT64_1.dll")
		); // dll loaded into this memory address

		// Pattern of bytes surrounding the instructions we are looking for,.
		// 0xFF denotes a wildcard
		BYTE targetBytes[] = {

			0x80, 0xBE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // cmp
			0x75, 0xFF,								  // jnz
			0x80, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // cmp
			0x75, 0xFF,								  // jnz	// Target!
			0xC7, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mov
			0x33, 0xFF,								  // xor
			0xE9, 0xFF, 0xFF, 0xFF, 0xFF			  // jmp

		};

		// First byte we're modifying.
		// Easier to copy-paste than keep counting the offset when I change things.
		const BYTE targetInstruction[] = {0x75,
										  0xFF,
										  0xC7,
										  0x45,
										  0xFF,
										  0xFF,
										  0xFF,
										  0xFF,
										  0xFF,
										  0x33,
										  0xFF,
										  0xE9,
										  0xFF,
										  0xFF,
										  0xFF,
										  0xFF};

		const BYTE replacementInstruction = 0x74; // Byte we're replacing it with

		// Location of target byte relative to first byte of search pattern
		int targetOffset = 0;
		for (int i = 0; i < std::size(targetBytes); i++)
		{
			if (targetBytes[i] == targetInstruction[0]
				&& targetBytes[i + 1] == targetInstruction[1])
			{
				targetOffset = i;
				break;
			}
		}

		// Look up module size to limit search range
		MODULEINFO moduleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo)))
		{
			std::cerr << "GetModuleInformation failed";
		}

		size_t moduleSize = moduleInfo.SizeOfImage;
		BYTE* baseAddress = reinterpret_cast<BYTE*>(hModule);
		BYTE* targetAddress = baseAddress;

		int foundBytes = 0;
		int maxBytesFound = 0;
		bool found = false;
		BYTE* bestResultAddress = nullptr;
		BYTE* currentResultStartAddress = nullptr;

		// Loop through the module and find best match for pattern
		while (targetAddress < baseAddress + moduleSize)
		{
			if (*targetAddress == targetBytes[foundBytes] || targetBytes[foundBytes] == 0xFF)
			{
				if (foundBytes == 0)
				{
					currentResultStartAddress = targetAddress;
				}

				foundBytes++;

				if (foundBytes > maxBytesFound)
				{
					maxBytesFound = foundBytes;
					bestResultAddress = currentResultStartAddress;
				}

				targetAddress++;
			}
			else
			{
				if (foundBytes == 0)
				{
					// If we were not on the first byte of the pattern,
					// skip iterating address so the byte will be compared again
					// to the beginning of the pattern on the next loop
					targetAddress++;
				}

				foundBytes = 0;
			}

			if (foundBytes == std::size(targetBytes))
			{
				found = true;
				break;
			}
		}

		// Print bytes found, for reference
		if (bestResultAddress != nullptr)
		{
			std::cout << _T("Best match for pattern at: ") << static_cast<void*>(bestResultAddress)
					  << std::endl;

			BYTE* addr = bestResultAddress;
			for (int i = 0; i < maxBytesFound; i++)
			{
				std::cout << std::setw(2) << std::setfill('0') << std::hex
						  << static_cast<int>(*addr) << " ";
				addr++;
			}

			std::cout << std::endl;
		}

		if (found)
		{
			std::cout << "Found all bytes in pattern!" << std::endl;

			// modification target offset in search pattern
			targetAddress = bestResultAddress + targetOffset;
		}
		else
		{
			if (bestResultAddress != nullptr)
			{
				targetAddress = bestResultAddress + targetOffset;

				if (*bestResultAddress == targetInstruction[0])
				{
					targetAddress = bestResultAddress;
					found = true;

					std::cout << "Incomplete match but found expected instruction. Fingers crossed."
							  << std::endl;
				}
				else
				{
					std::cerr << "Could not find address to modify! Probably cannot read hand pose."
							  << std::endl;
				}
			}
		}

		if (found)
		{
			// Print the existing value
			BYTE currentValue = *targetAddress;
			std::cout << ("Existing value at offset ") << static_cast<void*>(targetAddress)
					  << (": 0x") << std::hex << static_cast<int>(currentValue) << std::endl;

			// Nuke protection so we can write to it
			DWORD oldProtect;
			if (VirtualProtect(
					static_cast<LPVOID>(targetAddress),
					sizeof(char),
					PAGE_EXECUTE_READWRITE,
					&oldProtect
				))
			{
				// Modify instruction
				*targetAddress = replacementInstruction;

				if (FlushInstructionCache(currentProcess, baseAddress, moduleSize))
				{
					std::cout << _T("Flushed instruction cache successfully") << std::endl;
				}
				else
				{
					std::cerr << "Failed to flush instruction cache!" << std::endl;
				}

				// Restore the original protection
				VirtualProtect(
					static_cast<LPVOID>(targetAddress), sizeof(char), oldProtect, &oldProtect
				);

				std::cout << _T("Instruction modified successfully") << std::endl;
			}
			else
			{
				std::cout << _T("Failed to change memory protection.") << std::endl;
			}
		}
		else
		{
			std::cerr << "Failed to find instruction in OVR dll!" << std::endl;
		}

		return;
	}

} // namespace HOL::hacks
