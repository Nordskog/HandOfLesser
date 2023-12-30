#include "HandOfLesser.h"
#include "core/HandOfLesserCore.h"
#include <HandOfLesserCommon.h>

int main(int /* argc */, char* /* argv */[])
{
	try
	{
		HandOfLesserCore app;
		app.init(9005);
		app.start();
	}
	catch (std::exception exp)
	{
		std::cerr << exp.what() << std::endl;
	}
}
