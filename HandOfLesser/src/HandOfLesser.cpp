#include "HandOfLesser.h"
#include "core/HandOfLesserCore.h"
#include <HandOfLesserCommon.h>

int main(int /* argc */, char* /* argv */[])
{
	HOL::HandOfLesserCore app;

	try
	{	
		app.init(9005);
		app.start();
	}
	catch (std::exception exp)
	{
		std::cerr << exp.what() << std::endl;
	}
}
