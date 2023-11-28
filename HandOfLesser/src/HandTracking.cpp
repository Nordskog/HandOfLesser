#include "HandTracking.h"
#include "HandTrackingInterface.h"
#include <HandOfLesserCommon.h>
#include "simple_gesture_detector.h"
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace HOL;
using namespace HOL::SimpleGesture;

void HandTracking::init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::init(instance);
	this->initHands(session);
}

void HandTracking::initHands(xr::UniqueDynamicSession& session)
{
	this->mLeftHand = std::make_unique<OpenXRHand>(session, XrHandEXT::XR_HAND_LEFT_EXT);
	this->mRightHand = std::make_unique<OpenXRHand>(session, XrHandEXT::XR_HAND_RIGHT_EXT);
}

void HandTracking::updateHands(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mLeftHand->updateJointLocations(space, time);
	this->mRightHand->updateJointLocations(space, time);
}

void HandTracking::updateInputs()
{
	updateSimpleGestures();
}

void HandTracking::updateSimpleGestures()
{
	HOL::SimpleGesture::populateGestures(
		this->mLeftHand.get()->simpleGestures, this->mLeftHand.get()
	);
	HOL::SimpleGesture::populateGestures(
		this->mRightHand.get()->simpleGestures, this->mRightHand.get()
	);
}

OpenXRHand* HandTracking::getHand(XrHandEXT side)
{
	if (side == XrHandEXT::XR_HAND_LEFT_EXT)
	{
		return this->mLeftHand.get();
	}
	else
	{
		return this->mRightHand.get();
	}
}

HOL::HandTransformPacket HandTracking::getTransformPacket(XrHandEXT side)
{
	OpenXRHand* hand = getHand(side);

	HOL::HandTransformPacket packet;

	packet.valid = hand->handPose.poseValid;
	packet.side = (HOL::HandSide)side;
	packet.location = hand->handPose.palmLocation;
	packet.velocity = hand->handPose.palmVelocity;

	return packet;
}

HOL::ControllerInputPacket HandTracking::getInputPacket(XrHandEXT side)
{
	// todo we're replacing all of this
	OpenXRHand* hand = getHand(side);

	HOL::ControllerInputPacket packet;

	packet.valid = hand->handPose.poseValid;
	packet.side = (HOL::HandSide)side;

	packet.triggerClick = hand->simpleGestures[SimpleGestureType::IndexFingerPinch].click;
	packet.systemClick = hand->simpleGestures[SimpleGestureType::OpenHandFacingFace].click;

	//
	packet.fingerCurlIndex
		= mapCurlToSteamVR(hand->handPose.fingers[FingerType::FingerIndex].getCurlSum());
	packet.fingerCurlMiddle
		= mapCurlToSteamVR(hand->handPose.fingers[FingerType::FingerMiddle].getCurlSum());
	packet.fingerCurlRing
		= mapCurlToSteamVR(hand->handPose.fingers[FingerType::FingerRing].getCurlSum());
	packet.fingerCurlPinky
		= mapCurlToSteamVR(hand->handPose.fingers[FingerType::FingerPinky].getCurlSum());

	return packet;
}
