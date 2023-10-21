#include "HandOfLesserCore.h"
#include <thread>

void HandOfLesserCore::init(int serverPort)
{
	this->mInstanceHolder = std::make_unique<InstanceHolder>();
	this->mInstanceHolder->init();
	this->mInstanceHolder->beginSession();

	this->mHandTracking = std::make_unique<HandTracking>();
	this->mHandTracking->init(this->mInstanceHolder->mInstance, this->mInstanceHolder->mSession);

	this->mTransport.init(serverPort);

	this->mUserInterface = std::make_unique<UserInterface>();
	this->mUserInterface.get()->init();
}

void HandOfLesserCore::start()
{
	this->mainLoop();
}

std::vector<const char*> HandOfLesserCore::getRequiredExtensions()
{
	return std::vector<const char*>();
}

void HandOfLesserCore::mainLoop()
{
	while (1)
	{
		doOpenXRStuff();

		this->mUserInterface.get()->onFrame();
		if (this->mUserInterface.get()->shouldClose())
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	std::cout << "Exiting loop" << std::endl;
}

void HandOfLesserCore::doOpenXRStuff()
{
	XrTime time = this->mInstanceHolder.get()->getTime();
	time += 1000000 * 32;

	this->mInstanceHolder.get()->pollEvent();

	this->mHandTracking->updateHands(this->mInstanceHolder->mStageSpace, time);
	this->mHandTracking->updateInputs();

	this->sendUpdate();

	return;
}

void HandOfLesserCore::sendUpdate()
{
	{
		HOL::HandTransformPacket packet
			= this->mHandTracking->getTransformPacket(XrHandEXT::XR_HAND_LEFT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));

		packet = this->mHandTracking->getTransformPacket(XrHandEXT::XR_HAND_RIGHT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));
	}

	{
		HOL::ControllerInputPacket packet
			= this->mHandTracking->getInputPacket(XrHandEXT::XR_HAND_LEFT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));

		packet = this->mHandTracking->getInputPacket(XrHandEXT::XR_HAND_RIGHT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));
	}
}
