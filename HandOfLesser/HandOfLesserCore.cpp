#include "HandOfLesserCore.h"

void HandOfLesserCore::init()
{
	this->mInstanceHolder = std::make_unique<InstanceHolder>();
	this->mInstanceHolder->init();
	this->mInstanceHolder->beginSession();

	this->mHandTracking = std::make_unique<HandTracking>();
	this->mHandTracking->init(this->mInstanceHolder->mInstance, this->mInstanceHolder->mSession);
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

	this->mHandTracking->updateHands( this->mInstanceHolder->mLocalSpace, time );
}