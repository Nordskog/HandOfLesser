#include "HandTracking.h"
#include "HandTrackingInterface.h"
#include <HandOfLesserCommon.h>
#include "src/hands/simple_gesture_detector.h"
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace HOL;
using namespace HOL::OpenXR;
using namespace HOL::SimpleGesture;

void HandTracking::init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::init(instance);
	this->initHands(session);
}

void HandTracking::initHands(xr::UniqueDynamicSession& session)
{
	this->mLeftHand.init(session, HOL::LeftHand);
	this->mRightHand.init(session, HOL::RightHand);
}

void HandTracking::updateHands(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mLeftHand.updateJointLocations(space, time);
	this->mRightHand.updateJointLocations(space, time);
}

void HandTracking::updateInputs()
{
	updateSimpleGestures();
}

void HandTracking::updateSimpleGestures()
{
	HOL::SimpleGesture::populateGestures(this->mLeftHand.simpleGestures, this->mLeftHand);
	HOL::SimpleGesture::populateGestures(this->mRightHand.simpleGestures, this->mRightHand);
}

OpenXRHand& HandTracking::getHand(HOL::HandSide side)
{
	if (side == HOL::HandSide::LeftHand)
	{
		return this->mLeftHand;
	}
	else
	{
		return this->mRightHand;
	}
}

HOL::HandTransformPacket HandTracking::getTransformPacket(HOL::HandSide side)
{
	OpenXRHand hand = getHand(side);

	HOL::HandTransformPacket packet;

	packet.active = hand.handPose.active;
	packet.valid = hand.handPose.poseValid;
	packet.stale = hand.handPose.poseStale;
	packet.side = (HOL::HandSide)side;
	packet.location = hand.handPose.palmLocation;
	packet.velocity = hand.handPose.palmVelocity;

	return packet;
}

HOL::ControllerInputPacket HandTracking::getInputPacket(HOL::HandSide side)
{
	// todo we're replacing all of this
	OpenXRHand hand = getHand(side);
	OpenXRHand otherHand = getHand(side == HOL::HandSide::LeftHand ? HOL::HandSide::RightHand
																   : HOL::HandSide::LeftHand);

	HOL::ControllerInputPacket packet;

	packet.valid = hand.handPose.poseValid;
	packet.side = (HOL::HandSide)side;

	// A temporary end to the accidentally opening menu madness. System gesture is terrible.
	if (side == HOL::HandSide::RightHand)
	{
		// Require both hands to do the thing, only trigger on right hand
		packet.systemClick
			= hand.simpleGestures[SimpleGestureType::OpenHandFacingFace].click
			  && otherHand.simpleGestures[SimpleGestureType::OpenHandFacingFace].click;
	}
	else
	{
		packet.systemClick = false;
	}

	packet.triggerClick = hand.simpleGestures[SimpleGestureType::IndexFingerPinch].click;

	//
	packet.fingerCurlIndex
		= mapCurlToSteamVR(hand.handPose.fingers[FingerType::FingerIndex].getCurlSum());
	packet.fingerCurlMiddle
		= mapCurlToSteamVR(hand.handPose.fingers[FingerType::FingerMiddle].getCurlSum());
	packet.fingerCurlRing
		= mapCurlToSteamVR(hand.handPose.fingers[FingerType::FingerRing].getCurlSum());
	packet.fingerCurlPinky
		= mapCurlToSteamVR(hand.handPose.fingers[FingerType::FingerLittle].getCurlSum());

	// map index curl to trigger touch and force
	float triggerValueRange = 0.15f;
	packet.triggerValue = std::clamp(
		(packet.fingerCurlIndex - (1.0f - triggerValueRange)) / triggerValueRange, 0.0f, 1.0f);
	packet.triggerTouch = packet.fingerCurlIndex >= (1.0f - triggerValueRange);

	// steamvr will not allow a click unless the triggerValue is 1'ish
	// does touch matter? who knows
	if (packet.triggerClick)
	{
		packet.triggerValue = 1.0f;
		packet.triggerTouch = true;
	}

	float gripRaw // use average of remaining finger's curl for grip
		= (packet.fingerCurlMiddle + packet.fingerCurlRing + packet.fingerCurlPinky) / 3.0f;

	// Map rest of fingers to grip touch and force
	// Vrchat jumps to fist pose as soon as there is any force
	float gripForceRange = 0.01f; // Just jump to 1 at full bend
	float gripValueRange = 0.3f;
	packet.gripValue = std::clamp((gripRaw - (1.0f - gripValueRange)) / gripValueRange, 0.0f, 1.0f);
	packet.gripForce = std::clamp((gripRaw - (1.0f - gripForceRange)) / gripForceRange, 0.0f, 1.0f);
	packet.gripTouch = gripRaw >= (1.0f - gripValueRange);

	return packet;
}

HOL::HandPose& HandTracking::getHandPose(HOL::HandSide side)
{
	OpenXRHand& hand = (side == HOL::LeftHand) ? this->mLeftHand : this->mRightHand;
	return hand.handPose;
}
