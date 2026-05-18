#include "HandOfLesser.h"
#include "core/HandOfLesserCore.h"
#include "steamvr/steamvr_setup.h"
#include <HandOfLesserCommon.h>

int main(int argc, char* argv[])
{
	int exitCode = 0;
	if (HOL::SteamVR::handleUtilityCommandLine(argc, argv, exitCode))
	{
		return exitCode;
	}

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
			std::cerr << "Fatal error: " << ex.what() << std::endl;
			shouldRestart = false;
		}
		catch (...)
		{
			std::cerr << "Fatal error: unknown exception" << std::endl;
			shouldRestart = false;
		}
#endif
	}
	while (shouldRestart);

	return 0;
}
