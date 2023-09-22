#include "TrackedHand.h"
#include "HandTrackingInterface.h"

#include <iostream>
#include <utility>

TrackedHand::TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	init(session, side);
}

void TrackedHand::init( xr::UniqueDynamicSession& session,  XrHandEXT side )
{
	this->mSide = side;
	HandTrackingInterface::createHandTracker(session, side, this->mHandTracker);
}

void TrackedHand::updateJointLocations( xr::UniqueDynamicSpace& space, XrTime time ) 
{
	HandTrackingInterface::locateHandJoints(this->mHandTracker, space, time, this->mJointocations);
}

HOL::HandTransformPacket TrackedHand::getNativePacket()
{
	HOL::HandTransformPacket packet;
	if (this->mSide == XrHandEXT::XR_HAND_LEFT_EXT)
	{
		packet.hand = vr::TrackedControllerRole_LeftHand;
	}
	else if (this->mSide == XrHandEXT::XR_HAND_RIGHT_EXT)
	{
		packet.hand = vr::TrackedControllerRole_RightHand;
	}
	else
	{
		packet.hand = vr::TrackedControllerRole_Invalid;
	}

	auto hand = this->mJointocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	packet.position.v[0] = hand.pose.position.x;
	packet.position.v[1] = hand.pose.position.y;
	packet.position.v[2] = hand.pose.position.z;

	packet.qRotation.w = hand.pose.orientation.w;
	packet.qRotation.x = hand.pose.orientation.x;
	packet.qRotation.y = hand.pose.orientation.y;
	packet.qRotation.z = hand.pose.orientation.z;

	return packet;
}

