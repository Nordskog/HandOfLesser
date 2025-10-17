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
						hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					std::cout << "Checking dll: " << szModName << std::endl;

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

	void applyPatch(HMODULE hModule,
					const BYTE* targetBytes,
					size_t targetByteSize,
					const BYTE* targetInstruction,
					size_t targetInstructionSize,
					const BYTE* replacementInstruction,
					size_t replacementInstructionSize)
	{
		HANDLE currentProcess = GetCurrentProcess();

		// Location of target byte relative to first byte of search pattern


		int targetOffset = targetByteSize - targetInstructionSize;

		// Look up module size to limit search range
		MODULEINFO moduleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo)))
		{
			std::cerr << "GetModuleInformation failed";
		}

		size_t moduleSize = moduleInfo.SizeOfImage;

		BYTE* baseAddress = reinterpret_cast<BYTE*>(hModule);
		BYTE* endAddress = baseAddress + moduleInfo.SizeOfImage;
		BYTE* targetAddress = baseAddress;

		int foundBytes = 0;
		int maxBytesFound = 0;
		bool found = false;
		BYTE* bestResultAddress = nullptr;
		BYTE* currentResultStartAddress = nullptr;

		MEMORY_BASIC_INFORMATION mbi;

		// Loop through the module and find best match for pattern
		while (targetAddress < endAddress)
		{
			if (!VirtualQuery(targetAddress, &mbi, sizeof(mbi)))
				break;

			// Skip non-committed or non-readable regions
			if (mbi.State != MEM_COMMIT
				|| !(mbi.Protect
					 & (PAGE_EXECUTE_READ | PAGE_READONLY | PAGE_READWRITE
						| PAGE_EXECUTE_READWRITE)))
			{
				targetAddress = static_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
				continue;
			}

			BYTE* regionEnd = targetAddress + mbi.RegionSize;
			if (regionEnd > endAddress)
				regionEnd = endAddress;

			while (targetAddress < regionEnd)
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

						BYTE* addr = bestResultAddress;
						for (int i = 0; i < maxBytesFound; i++)
						{
							std::cout << std::setw(2) << std::setfill('0') << std::hex
									  << static_cast<int>(*addr) << " ";
							addr++;
						}
						std::cout << std::endl;
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

				if (foundBytes == targetByteSize)
				{
					found = true;
					break;
				}
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

				if (*targetAddress == targetInstruction[0])
				{
					found = true;

					std::cout << "Incomplete match but found expected instruction. Fingers crossed."
							  << std::endl;
				}
				else
				{
					std::cerr << "Could not find address to modify! No changes were made."
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
			if (VirtualProtect(static_cast<LPVOID>(targetAddress),
							   sizeof(char) * replacementInstructionSize,
							   PAGE_EXECUTE_READWRITE,
							   &oldProtect))
			{
				// Modify instruction
				memcpy(targetAddress, replacementInstruction, replacementInstructionSize);

				if (FlushInstructionCache(currentProcess, baseAddress, moduleSize))
				{
					std::cout << _T("Flushed instruction cache successfully") << std::endl;
				}
				else
				{
					std::cerr << "Failed to flush instruction cache!" << std::endl;
				}

				// Restore the original protection
				VirtualProtect(static_cast<LPVOID>(targetAddress),
							   sizeof(char) * replacementInstructionSize,
							   oldProtect,
							   &oldProtect);

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

	/// <summary>
	/// Modifies ovr_GetHandPose LibOVRRT64_1.dll and inverts the clause that
	/// presumably checks if the session state is FOCUSED by replacing a jnz instruction with jz
	/// </summary>
	void fixOvrSessionStateRestriction()
	{
		std::cout << "Patching OVR session state restriction" << std::endl;

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

		const BYTE replacementInstruction[] = {0x74}; // Byte we're replacing it with

		HANDLE currentProcess = GetCurrentProcess();
		HMODULE hModule = getModule(
			currentProcess, _T("\LibOVRRTImpl64_1.dll")); // dll loaded into this memory address

		applyPatch(hModule,
				   targetBytes,
				   std::size(targetBytes),
				   targetInstruction,
				   std::size(targetInstruction),
				   replacementInstruction,
				   std::size(replacementInstruction));
	}

	/// <summary>
	/// Modifies XR_EXT_hand_tracking.dll and inverts the clause that
	/// does god only knows what and concludes that multimodal tracking is not supported
	/// </summary>
	void fixOvrMultimodalSupportCheck()
	{
		std::cout << "Patching OVR multimodal support check" << std::endl;

		// Pattern of bytes surrounding the instructions we are looking for,.
		// 0xFF denotes a wildcard
		BYTE targetBytes[] = {
			0x0F, 0x85, 0xFF, 0xFF, 0xFF, 0xFF,		  // JNZ rel32
			0x44, 0x38, 0x25, 0xFF, 0xFF, 0xFF, 0xFF, // CMP byte ptr [RIP+disp32]
			0x0F, 0x84, 0xFF, 0xFF, 0xFF, 0xFF,		  // JZ  rel32	// Target!
			0x48, 0x8B, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, // MOV reg,[RIP+disp32]
			0x48, 0x83, 0xC1, 0x08,					  // ADD rcx,8  (kept)
			0x48, 0x8B, 0x01,						  // MOV rax,[rcx]
			0x4C, 0x8B, 0xC3,						  // MOV r8,rbx
			0x48, 0x8B, 0x55, 0x80,					  // MOV rdx,[rbp-0x80]  (kept)
			0xFF, 0x50, 0x08						  // CALL qword ptr [reg+0x08] (kept)

		};

		// First byte we're modifying.
		// Easier to copy-paste than keep counting the offset when I change things.
		const BYTE targetInstruction[]
			= {0x84, 0xFF, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x0D, 0xFF, 0xFF,
			   0xFF, 0xFF, 0x48, 0x83, 0xC1, 0x08, 0x48, 0x8B, 0x01, 0x4C,
			   0x8B, 0xC3, 0x48, 0x8B, 0x55, 0x80, 0xFF, 0x50, 0x08};

		const BYTE replacementInstruction[] = {0x85}; // Byte we're replacing it with

		HANDLE currentProcess = GetCurrentProcess();
		HMODULE hModule = getModule(
			currentProcess, _T("\XR_EXT_hand_tracking.dll")); // dll loaded into this memory address

		applyPatch(hModule,
				   targetBytes,
				   std::size(targetBytes),
				   targetInstruction,
				   std::size(targetInstruction),
				   replacementInstruction,
				   std::size(replacementInstruction));
	}

} // namespace HOL::hacks
