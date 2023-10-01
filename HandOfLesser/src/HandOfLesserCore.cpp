#include "HandOfLesserCore.h"

void HandOfLesserCore::init( int serverPort )
{
	this->mInstanceHolder = std::make_unique<InstanceHolder>();
	this->mInstanceHolder->init();
	this->mInstanceHolder->beginSession();

	this->mHandTracking = std::make_unique<HandTracking>();
	this->mHandTracking->init(this->mInstanceHolder->mInstance, this->mInstanceHolder->mSession);

	this->mTransport.init(serverPort);
}

void HandOfLesserCore::start()
{
	this->mInstanceHolder->setCallback(this);
	this->mInstanceHolder->mainLoop();
}

std::vector<const char*> HandOfLesserCore::getRequiredExtensions()
{
	return std::vector<const char*>();
}

void HandOfLesserCore::onFrame( XrTime time )
{
	time += 1000000 * 32;

	this->mHandTracking->updateHands( this->mInstanceHolder->mStageSpace, time );
	this->mHandTracking->updateInputs();

	this->sendUpdate();

}

void HandOfLesserCore::sendUpdate()
{
	{
		HOL::HandTransformPacket packet = this->mHandTracking->getTransformPacket(XrHandEXT::XR_HAND_LEFT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));

		packet = this->mHandTracking->getTransformPacket(XrHandEXT::XR_HAND_RIGHT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));
	}

	{
		HOL::ControllerInputPacket packet = this->mHandTracking->getInputPacket(XrHandEXT::XR_HAND_LEFT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));

		packet = this->mHandTracking->getInputPacket(XrHandEXT::XR_HAND_RIGHT_EXT);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));
	}




}
