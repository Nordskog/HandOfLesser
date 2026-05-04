#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace HOL
{
	class AppLauncher
	{
	public:
		void start();
		void stop();

	private:
		void launchThread();
		std::string getDefaultAppPath() const;
		bool isProcessRunning(const char* processName) const;
		bool launchDetached(const std::string& appPath, std::string& error) const;

		std::thread mThread;
		std::mutex mMutex;
		std::condition_variable mStop;
		bool mShouldStop = false;
	};
} // namespace HOL
