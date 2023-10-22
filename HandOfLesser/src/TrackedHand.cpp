#include "TrackedHand.h"
#include "HandTrackingInterface.h"
#include "settings_global.h"
#include "display_global.h"
#include "math_utils.h"
#include <iostream>
#include <utility>

#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace HOL;
using namespace HOL::SimpleGesture;

TrackedHand::TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	init(session, side);
}

void TrackedHand::init(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	this->mSide = side;
	HandTrackingInterface::createHandTracker(session, side, this->mHandTracker);
}

void TrackedHand::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	HandTrackingInterface::locateHandJoints(
		this->mHandTracker,
		space,
		time,
		this->mJointLocations,
		this->mJointVelocities,
		&this->mAimState
	);

	auto palmLocation = this->mJointLocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];
	auto palmVelocity = this->mJointVelocities[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	// Orientation is not going to be set without position for hand tracking.
	this->mPoseValid = (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
					   == XR_SPACE_LOCATION_POSITION_VALID_BIT;
	/*
	if (this->mSide == XrHandEXT::XR_HAND_RIGHT_EXT)
	{
		printf("Index pinch: %d, Menu: %d, System: %d, Index %.2f, Middle: %.2f, Ring: %.2f, Little:
	%.2f\n",
			((this->mAimState.status & XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB) ==
	XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB), ( (this->mAimState.status &
	XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB) == XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB),
			((this->mAimState.status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB ) ==
	XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB), this->mAimState.pinchStrengthIndex,
			this->mAimState.pinchStrengthMiddle,
			this->mAimState.pinchStrengthRing,
			this->mAimState.pinchStrengthLittle

			);
	}*/

	this->mPalmVelocity.linearVelocity = HOL::toEigenVector(palmVelocity.linearVelocity);
	this->mPalmVelocity.angularVelocity = HOL::toEigenVector(palmVelocity.angularVelocity);

	// printf("%.2f, %.2f, %.2f\n", HOL::settings::OrientationOffset.x,
	// HOL::settings::OrientationOffset.y, HOL::settings::OrientationOffset.z);

	// Transform location to grip pose
	// left
	Eigen::Vector3f gripRotationOffset = Eigen::Vector3f(45 + 15, 0, -90 + 13);

	// left
	Eigen::Vector3f gripTranslationOffset = Eigen::Vector3f(0.021, 0.009, -0.097);

	// Transform location to grip pose
	Eigen::Vector3f mainRotationOffset = gripRotationOffset + HOL::settings::OrientationOffset;
	Eigen::Vector3f mainTranslationOffset = gripTranslationOffset + HOL::settings::PositionOffset;

	if (this->mSide == XR_HAND_LEFT_EXT)
	{
		// Base offsets are for the left hand
	}
	else
	{
		mainRotationOffset = flipHandRotation(mainRotationOffset);
		mainTranslationOffset = flipHandTranslation(mainTranslationOffset);
	}

	// printf("%.2f, %.2f, %.2f, %.2f\n", offset.x, offset.y, offset.z, offset.w);

	////////////
	// Rot
	///////////

	Eigen::Quaternionf rawOrientation = HOL::toEigenQuaternion(palmLocation.pose.orientation);
	Eigen::Quaternionf rotationOffsetQuat
		= HOL::quaternionFromEulerAnglesDegrees(mainRotationOffset);

	this->mPalmLocation.orientation = rawOrientation * rotationOffsetQuat;

	//////////////
	// Trans
	/////////////

	Eigen::Vector3f rawPosition = HOL::toEigenVector(palmLocation.pose.position);
	Eigen::Vector3f rotationOffsetLocal = this->mPalmLocation.orientation * mainTranslationOffset;
	this->mPalmLocation.position = rawPosition + rotationOffsetLocal;

	{
		HOL::display::HandTransform[this->mSide].rawPose.position = rawPosition;
		HOL::display::HandTransform[this->mSide].rawPose.orientation = rawOrientation;

		HOL::display::HandTransform[this->mSide].finalPose.position = this->mPalmLocation.position;
		HOL::display::HandTransform[this->mSide].finalPose.orientation = this->mPalmLocation.orientation;

		HOL::display::HandTransform[this->mSide].finalOffset.position = mainTranslationOffset;
		HOL::display::HandTransform[this->mSide].finalOffset.orientation = rotationOffsetQuat;
	}
}

HOL::HandTransformPacket TrackedHand::getTransformPacket()
{
	HOL::HandTransformPacket packet;

	packet.valid = this->mPoseValid;
	packet.side = (HOL::HandSide)this->mSide;
	packet.location = this->mPalmLocation;
	packet.velocity = this->mPalmVelocity;

	return packet;
}

HOL::ControllerInputPacket TrackedHand::getInputPacket()
{
	HOL::ControllerInputPacket packet;

	packet.valid = this->mPoseValid;
	packet.side = (HOL::HandSide)this->mSide;

	packet.trigger = this->mSimpleGestures[SimpleGestureType::IndexFingerPinch].value;
	packet.triggerClick = this->mSimpleGestures[SimpleGestureType::IndexFingerPinch].click;
	packet.systemClick = this->mSimpleGestures[SimpleGestureType::OpenHandFacingFace].click;

	const auto getJoint = [&](XrHandJointEXT joint) {
		return HOL::toEigenVector(this->mJointLocations[joint].pose.position);
	};

	packet.fingerCurlIndex = computeOpenXrCurl(
		getJoint(XrHandJointEXT::XR_HAND_JOINT_INDEX_METACARPAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_INDEX_PROXIMAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_INDEX_INTERMEDIATE_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_INDEX_DISTAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_INDEX_TIP_EXT)
	);
	packet.fingerCurlMiddle = computeOpenXrCurl(
		getJoint(XrHandJointEXT::XR_HAND_JOINT_MIDDLE_METACARPAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_MIDDLE_DISTAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_MIDDLE_TIP_EXT)
	);
	packet.fingerCurlRing = computeOpenXrCurl(
		getJoint(XrHandJointEXT::XR_HAND_JOINT_RING_METACARPAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_RING_PROXIMAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_RING_INTERMEDIATE_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_RING_DISTAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_RING_TIP_EXT)
	);
	packet.fingerCurlPinky = computeOpenXrCurl(
		getJoint(XrHandJointEXT::XR_HAND_JOINT_LITTLE_METACARPAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_LITTLE_PROXIMAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_LITTLE_DISTAL_EXT),
		getJoint(XrHandJointEXT::XR_HAND_JOINT_LITTLE_TIP_EXT)
	);

	std::cout
		<< "Side: " << this->mSide
		<< "; Index: " << packet.fingerCurlIndex
		<< "; Middle: " << packet.fingerCurlMiddle
		<< "; Ring: " << packet.fingerCurlRing
		<< "; Pinky: " << packet.fingerCurlPinky
		<< std::endl;

	return packet;
}
