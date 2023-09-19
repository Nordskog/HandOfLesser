#include "HandTracking.h"
#include "HandTrackingInterface.h"

void HandTracking::init( xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::init(instance);
	this->initHands(session);
}

void HandTracking::initHands(xr::UniqueDynamicSession& session)
{
	this->mLeftHand = std::make_unique<TrackedHand>(session, XrHandEXT::XR_HAND_LEFT_EXT);
	this->mRightHand = std::make_unique<TrackedHand>(session, XrHandEXT::XR_HAND_RIGHT_EXT);
}

void HandTracking::updateHands( xr::UniqueDynamicSpace& space, XrTime time )
{
	this->mLeftHand->updateJointLocations(space, time);
	this->mRightHand->updateJointLocations(space, time);
}