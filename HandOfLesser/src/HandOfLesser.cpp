#include "HandOfLesser.h"
#include "core/HandOfLesserCore.h"
#include "steamvr/steamvr_setup.h"
#include "windows/crash_handler.h"
#include "windows/console_log.h"
#include <HandOfLesserCommon.h>
#include <iostream>

int main(int argc, char* argv[])
{
	HOL::Windows::ConsoleLog::install();

	int exitCode = 0;
	if (HOL::SteamVR::handleUtilityCommandLine(argc, argv, exitCode))
	{
		return exitCode;
	}

#if !defined(_DEBUG)
	HOL::Windows::CrashHandler::install();
#endif

	bool shouldRestart = false;

	do
	{
		HOL::HandOfLesserCore app;

#if defined(_DEBUG)
		{
			app.init(9005);
			shouldRestart = app.start();
		}
#else
		try
		{
			app.init(9005);
			shouldRestart = app.start();
		}
		catch (const std::exception& ex)
		{
			HOL::Windows::CrashHandler::logHandledException(ex);
			shouldRestart = false;
		}
		catch (...)
		{
			HOL::Windows::CrashHandler::logHandledUnknownException();
			shouldRestart = false;
		}
#endif
	}
	while (shouldRestart);

	return 0;
}
