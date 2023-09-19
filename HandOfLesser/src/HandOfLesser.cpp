// HandOfLesser.cpp : Defines the entry point for the application.
//

#include "HandOfLesser.h"
#include "HandOfLesserCore.h"

int main(int /* argc */, char* /* argv */[])
{
    try
    {
        HandOfLesserCore app;
        app.init();
        app.start();

    }
    catch (std::exception exp)
    {
        std::cout << exp.what() << std::endl;
    }

}
