#include "TrackedHand.h"
#include "HandTrackingInterface.h"

#include <iostream>

TrackedHand::TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	init(session, side);
}

void TrackedHand::init( xr::UniqueDynamicSession& session,  XrHandEXT side )
{
	HandTrackingInterface::createHandTracker(session, side, this->mHandTracker);
}

void TrackedHand::updateJointLocations( xr::UniqueDynamicSpace& space, XrTime time ) 
{
	HandTrackingInterface::locateHandJoints(this->mHandTracker, space, time, this->mJointocations);

	auto hand = this->mJointocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];
	
	std::cout << "Hand: " << std::to_string(hand.pose.position.x).c_str() << std::endl;

}