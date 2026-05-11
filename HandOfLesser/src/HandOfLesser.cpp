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

		//try
		{
			app.init(9005);
			shouldRestart = app.start();
		}
		//catch (std::exception exp)
		{
			//std::cerr << exp.what() << std::endl;
		}
	}
	while (shouldRestart);

	return 0;
}
