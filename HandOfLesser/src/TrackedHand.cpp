#include "TrackedHand.h"
#include "HandTrackingInterface.h"
#include "settings_global.h"
#include "XrUtils.h"

#include <xr_linear.h>
#include <iostream>
#include <utility>

using namespace HOL;
using namespace HOL::SimpleGesture;

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
	/*
	if (this->mSide == XrHandEXT::XR_HAND_RIGHT_EXT)
	{
		printf("Index pinch: %d, Menu: %d, System: %d, Index %.2f, Middle: %.2f, Ring: %.2f, Little: %.2f\n",
			((this->mAimState.status & XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB) == XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB),
			( (this->mAimState.status & XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB) == XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB),
			((this->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB ) == XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB),
			this->mAimState.pinchStrengthIndex,
			this->mAimState.pinchStrengthMiddle,
			this->mAimState.pinchStrengthRing,
			this->mAimState.pinchStrengthLittle

			);
	}*/

	//printf("%.2f, %.2f, %.2f\n", HOL::settings::OrientationOffset.x, HOL::settings::OrientationOffset.y, HOL::settings::OrientationOffset.z);

	// Transform location to grip pose
	// left
	XrVector3f gripRotationOffset = XrVector3f(
		45 + 15,
		0,
		-90 + 13
	);

	// left
	XrVector3f gripTranslationOffset = XrVector3f(
		0.021,
		0.009,
		-0.097
	);

	// Transform location to grip pose
	XrVector3f mainRotationOffset;
	XrVector3f secondaryRotationOffset;

	XrVector3f mainTranslationOffset;
	XrVector3f secondaryTranslationOffset;

	if (this->mSide == XR_HAND_LEFT_EXT)
	{
		mainRotationOffset = gripRotationOffset;
		secondaryRotationOffset = HOL::settings::OrientationOffset;

		mainTranslationOffset = gripTranslationOffset;
		secondaryTranslationOffset = HOL::settings::PositionOffset;
	}
	else
	{
		mainRotationOffset = flipRotation(gripRotationOffset);
		secondaryRotationOffset = flipRotation(HOL::settings::OrientationOffset);

		mainTranslationOffset = flipTranslation(gripTranslationOffset);
		secondaryTranslationOffset = flipTranslation(HOL::settings::PositionOffset);
	}

	//printf("%.2f, %.2f, %.2f, %.2f\n", offset.x, offset.y, offset.z, offset.w);

	////////////
	// Rot
	///////////


	XrQuaternionf originalOrientation = this->mLocation.pose.orientation;
	XrVector3f rotationOffset;
	XrVector3f_Add(&rotationOffset, &mainRotationOffset, &secondaryRotationOffset);
	XrQuaternionf rotationOffsetQuat = quaternionFromEulerAnglesDegrees(rotationOffset);

	XrQuaternionf_Multiply(&this->mLocation.pose.orientation, &rotationOffsetQuat, &originalOrientation);

	//////////////
	// Trans
	/////////////

	XrVector3f originalPosition = this->mLocation.pose.position;
	XrVector3f positionOffset;
	XrVector3f rotatedPositionOffset;
	XrVector3f_Add(&positionOffset, &mainTranslationOffset, &secondaryTranslationOffset);

	XrQuaternionf_RotateVector3f(&rotatedPositionOffset, &this->mLocation.pose.orientation, &positionOffset);
	XrVector3f_Add(&this->mLocation.pose.position, &originalPosition, &rotatedPositionOffset);

	if (this->mSide == XR_HAND_LEFT_EXT)
	{
		HOL::settings::FinalOrientationOffsetLeft = rotationOffset;
		HOL::settings::FinalPositionOffsetLeft = positionOffset;
	}
	else
	{
		HOL::settings::FinalOrientationOffsetRight = rotationOffset;
		HOL::settings::FinalPositionOffsetRight = positionOffset;
	}

}

HOL::HandTransformPacket TrackedHand::getTransformPacket()
{
	HOL::HandTransformPacket packet;

	packet.valid = this->mPoseValid;
	packet.side = this->mSide;
	packet.location = this->mLocation;
	packet.velocity = this->mVelocity;

	return packet;
}

HOL::ControllerInputPacket TrackedHand::getInputPacket()
{
	HOL::ControllerInputPacket packet;

	packet.valid = this->mPoseValid;
	packet.side = this->mSide;

	packet.trigger = this->mSimpleGestures[SimpleGestureType::IndexFingerPinch].value;
	packet.triggerClick = this->mSimpleGestures[SimpleGestureType::IndexFingerPinch].click;
	packet.systemClick = this->mSimpleGestures[SimpleGestureType::OpenHandFacingFace].click;

	/*
	if (this->mSide == XrHandEXT::XR_HAND_RIGHT_EXT)
	{
		printf("Trigger: %d, System: %d\n",
			this->mSimpleGestures[SimpleGestureType::IndexFingerPinch].click,
			this->mSimpleGestures[SimpleGestureType::OpenHandFacingFace].click
		);
	}*/



	return packet;
}

