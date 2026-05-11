#include "steamvr_setup.h"
#include <openvr.h>
#include <cstring>
#include <iostream>
#include <string>

namespace
{
	int activateMultipleDrivers()
	{
		int exitCode = -2;
		vr::EVRInitError initError = vr::VRInitError_None;
		vr::VR_Init(&initError, vr::VRApplication_Utility);
		if (initError != vr::VRInitError_None)
		{
			std::cerr << "Failed to initialize OpenVR: "
					  << vr::VR_GetVRInitErrorAsEnglishDescription(initError) << std::endl;
			vr::VR_Shutdown();
			return exitCode;
		}

		vr::EVRSettingsError settingsError = vr::VRSettingsError_None;
		bool enabled = vr::VRSettings()->GetBool(vr::k_pch_SteamVR_Section,
												 vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool,
												 &settingsError);
		if (settingsError != vr::VRSettingsError_None)
		{
			std::cerr << "Could not read \"" << vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool
					  << "\" setting: "
					  << vr::VRSettings()->GetSettingsErrorNameFromEnum(settingsError)
					  << std::endl;
			vr::VR_Shutdown();
			return -3;
		}

		if (!enabled)
		{
			// Our controller/tracker driver runs alongside the active HMD driver, so SteamVR
			// must allow multiple drivers to be active at the same time.
			vr::VRSettings()->SetBool(vr::k_pch_SteamVR_Section,
									 vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool,
									 true,
									 &settingsError);
			if (settingsError != vr::VRSettingsError_None)
			{
				std::cerr << "Could not set \"" << vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool
						  << "\" setting: "
						  << vr::VRSettings()->GetSettingsErrorNameFromEnum(settingsError)
						  << std::endl;
				vr::VR_Shutdown();
				return -4;
			}

			std::cout << "Enabled \"" << vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool
					  << "\" setting" << std::endl;
		}
		else
		{
			std::cout << "\"" << vr::k_pch_SteamVR_ActivateMultipleDrivers_Bool
					  << "\" setting already enabled" << std::endl;
		}

		vr::VR_Shutdown();
		return 0;
	}
} // namespace

bool HOL::SteamVR::handleUtilityCommandLine(int argc, char* argv[], int& exitCode)
{
	if (argc < 2)
	{
		return false;
	}

	if (std::strcmp(argv[1], "-activatemultipledrivers") == 0)
	{
		exitCode = activateMultipleDrivers();
		return true;
	}

	return false;
}
