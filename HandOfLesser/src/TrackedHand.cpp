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
	HandTrackingInterface::locateHandJoints(this->mHandTracker, space, time, this->mJointocations, this->mJointVelocities, &this->mAimState);

	this->mLocation = this->mJointocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];
	this->mVelocity = this->mJointVelocities[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	// Orientation is not going to be set without position for hand tracking.
	this->mPoseValid = (this->mLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) == XR_SPACE_LOCATION_POSITION_VALID_BIT;
	
	if (this->mSide == XrHandEXT::XR_HAND_RIGHT_EXT)
	{
		printf("Menu: %d, System: %d, Index %.2f, Middle: %.2f, Ring: %.2f, Little: %.2f\n",
			( (this->mAimState.status & XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB) == XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB),
			((this->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB ) == XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB),
			this->mAimState.pinchStrengthIndex,
			this->mAimState.pinchStrengthMiddle,
			this->mAimState.pinchStrengthRing,
			this->mAimState.pinchStrengthLittle

			);
	}
}

HOL::HandTransformPacket TrackedHand::getNativePacket()
{
	HOL::HandTransformPacket packet;

	packet.valid = this->mPoseValid;
	packet.side = this->mSide;
	packet.location = this->mLocation;
	packet.velocity = this->mVelocity;
	if (packet.valid)
	{
		packet.inputs.trigger = this->mAimState.pinchStrengthIndex;
		packet.inputs.triggerClick = packet.inputs.trigger >= 1.0f;

		// Hand infront of face with open hand
		if (((this->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB) == XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB))
		{
			
			packet.inputs.systemClick = true;
		}
	}
	else
	{
		packet.inputs.trigger = 0;
		packet.inputs.triggerClick = false;

		packet.inputs.systemClick = false;
	}


	return packet;
}

