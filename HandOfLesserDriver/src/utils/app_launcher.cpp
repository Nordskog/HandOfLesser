#include "app_launcher.h"

#include <driverlog.h>
#include "openvr_driver.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <chrono>
#include <filesystem>

namespace HOL
{
	namespace
	{
		constexpr int AutoLaunchDelayMs = 3000;

		bool equalsIgnoreCase(const char* left, const char* right)
		{
			return _stricmp(left, right) == 0;
		}
	} // namespace

	void AppLauncher::start()
	{
		vr::EVRSettingsError error = vr::VRSettingsError_None;
		bool shouldLaunch
			= vr::VRSettings()->GetBool("driver_00handoflesser", "autoLaunchApp", &error);
		if (error != vr::VRSettingsError_None || !shouldLaunch)
		{
			DriverLog("App auto-launch disabled or unavailable. enabled=%d, settingsError=%d",
					  shouldLaunch,
					  error);
			return;
		}

		DriverLog("App auto-launch enabled; launch will be attempted in %dms", AutoLaunchDelayMs);
		mShouldStop = false;
		mThread = std::thread(&AppLauncher::launchThread, this);
	}

	void AppLauncher::stop()
	{
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mShouldStop = true;
		}

		mStop.notify_all();
		if (mThread.joinable())
		{
			mThread.join();
		}
	}

	void AppLauncher::launchThread()
	{
		// Launching immediately while SteamVR is still initializing can race foreground OpenXR
		// runtimes. The delay gives SteamVR time to settle before the app creates any session.
		std::unique_lock<std::mutex> lock(mMutex);
		if (mStop.wait_for(
				lock, std::chrono::milliseconds(AutoLaunchDelayMs), [this] { return mShouldStop; }))
		{
			return;
		}
		lock.unlock();

		if (isProcessRunning("HandOfLesser.exe"))
		{
			DriverLog("HandOfLesser app already running, skipping auto-launch");
			return;
		}

		std::string appPath = getDefaultAppPath();
		std::string launchError;
		if (!launchDetached(appPath, launchError))
		{
			DriverLog("Failed to auto-launch HandOfLesser app at '%s': %s",
					  appPath.c_str(),
					  launchError.c_str());
			return;
		}

		DriverLog("Auto-launched HandOfLesser app: %s", appPath.c_str());
	}

	std::string AppLauncher::getDefaultAppPath() const
	{
		char resourcePath[MAX_PATH]{};
		uint32_t requiredSize = vr::VRResources()->GetResourceFullPath(
			"{00handoflesser}/bin/win64/HandOfLesser.exe", "", resourcePath, sizeof(resourcePath));
		if (requiredSize > 0 && requiredSize <= sizeof(resourcePath))
		{
			DriverLog("Resolved HandOfLesser app path through SteamVR resources: %s", resourcePath);
			return resourcePath;
		}

		DriverLog("SteamVR resource path resolution failed for HandOfLesser.exe. requiredSize=%u",
				  requiredSize);
		return {};
	}

	bool AppLauncher::isProcessRunning(const char* processName) const
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
		{
			DriverLog("Failed to create process snapshot while checking for %s", processName);
			return false;
		}

		PROCESSENTRY32 entry{};
		entry.dwSize = sizeof(entry);

		bool found = false;
		if (Process32First(snapshot, &entry))
		{
			do
			{
				if (equalsIgnoreCase(entry.szExeFile, processName))
				{
					found = true;
					break;
				}
			}
			while (Process32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
		DriverLog("Process check for %s: %s", processName, found ? "running" : "not running");
		return found;
	}

	bool AppLauncher::launchDetached(const std::string& appPath, std::string& error) const
	{
		if (!std::filesystem::exists(appPath))
		{
			error = "app executable not found";
			DriverLog("Auto-launch target does not exist: %s", appPath.c_str());
			return false;
		}

		std::filesystem::path workingDirectory = std::filesystem::path(appPath).parent_path();

		STARTUPINFOA startupInfo{};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo{};
		BOOL launched = CreateProcessA(appPath.c_str(),
									   nullptr,
									   nullptr,
									   nullptr,
									   FALSE,
									   0,
									   nullptr,
									   workingDirectory.string().c_str(),
									   &startupInfo,
									   &processInfo);
		if (!launched)
		{
			error = "CreateProcessA failed";
			DriverLog("CreateProcessA failed for HandOfLesser.exe. LastError=%lu", GetLastError());
			return false;
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		return true;
	}
} // namespace HOL
