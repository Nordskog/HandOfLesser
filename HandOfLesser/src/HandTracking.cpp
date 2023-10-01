#include "HandTracking.h"
#include "HandTrackingInterface.h"
#include <HandOfLesserCommon.h>
#include "simple_gesture_detector.h"

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

void HandTracking::updateInputs()
{
	updateSimpleGestures();
}

void HandTracking::updateSimpleGestures()
{
	HOL::SimpleGesture::populateGestures(this->mLeftHand.get()->mSimpleGestures, this->mLeftHand.get());
	HOL::SimpleGesture::populateGestures(this->mRightHand.get()->mSimpleGestures, this->mRightHand.get());
}

HOL::HandTransformPacket HandTracking::getTransformPacket(XrHandEXT side)
{
	if (side == XrHandEXT::XR_HAND_LEFT_EXT)
	{
		return this->mLeftHand.get()->getTransformPacket();
	}
	else 
	{
		return this->mRightHand.get()->getTransformPacket();
	}
}

HOL::ControllerInputPacket HandTracking::getInputPacket(XrHandEXT side)
{
	if (side == XrHandEXT::XR_HAND_LEFT_EXT)
	{
		return this->mLeftHand.get()->getInputPacket();
	}
	else
	{
		return this->mRightHand.get()->getInputPacket();
	}
}
