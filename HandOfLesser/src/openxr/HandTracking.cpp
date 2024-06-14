#include "HandTracking.h"
#include "HandTrackingInterface.h"
#include <HandOfLesserCommon.h>
#include "src/hands/simple_gesture_detector.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include "src/core/ui/user_interface.h"
#include "XrUtils.h"
#include "src/core/settings_global.h"
#include "xr_hand_utils.h"

#include "src/hands/input/osc_axis_input.h"
#include "src/hands/input/settings_toggle_input.h"
#include "src/hands/action/button_action.h"
#include "src/hands/gesture/chain_gesture.h"

using namespace HOL;
using namespace HOL::OpenXR;
using namespace HOL::SimpleGesture;
using namespace std::chrono_literals;

void HandTracking::init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::init(instance);
	this->initHands(session);
	initGestures();
}

void HandTracking::initHands(xr::UniqueDynamicSession& session)
{
	this->mLeftHand.init(session, HOL::LeftHand);
	this->mRightHand.init(session, HOL::RightHand);
}

void HOL::OpenXR::HandTracking::initGestures()
{
	{
		auto handDragAction = HandDragAction::Create(HandSide::LeftHand,
													 XrHandJointEXT::XR_HAND_JOINT_THUMB_TIP_EXT);

		auto triggerGesture = OpenHandPinchGesture::Create();
		triggerGesture->setup(FingerType::FingerMiddle, HandSide::LeftHand);

		auto holdGesture = AimStateGesture::Create();
		holdGesture->setup(FingerType::FingerMiddle, HandSide::LeftHand);

		ActionParameters actionParams = handDragAction->getParameters();

		handDragAction->setTriggerGesture(triggerGesture);
		handDragAction->setHoldGesture(holdGesture);
		handDragAction->setSink(OscAxisInput::Create());

		this->mActions.push_back(handDragAction);
	}

	{
		auto chainGesture = ChainGesture::Create();

		std::vector<HOL::FingerType> allPinchFingers = {FingerType::FingerIndex,
														FingerType::FingerMiddle,
														FingerType::FingerRing,
														FingerType::FingerLittle};
		std::reverse(allPinchFingers.begin(),
					 allPinchFingers.end()); // Let's do from pinky to index

		for (auto otherFinger : allPinchFingers)
		{
			auto triggerGesture = OpenHandPinchGesture::Create();
			triggerGesture->setup(otherFinger, HandSide::RightHand);
			chainGesture->addGesture(triggerGesture);
		}

		auto settingsToggleInput = SettingsToggleInput::Create();
		settingsToggleInput->setup(HolSetting::SendOscInput);

		auto buttonAction = ButtonAction::Create();
		buttonAction->setTriggerGesture(chainGesture);
		buttonAction->setSink(settingsToggleInput);

		this->mActions.push_back(buttonAction);
	}
}

void HandTracking::updateHands(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mLeftHand.updateJointLocations(space, time);
	this->mRightHand.updateJointLocations(space, time);
}

void HandTracking::updateInputs()
{
	updateSimpleGestures();
	updateGestures();
}

void HandTracking::updateSimpleGestures()
{
	HOL::SimpleGesture::populateGestures(this->mLeftHand.simpleGestures, this->mLeftHand);
	HOL::SimpleGesture::populateGestures(this->mRightHand.simpleGestures, this->mRightHand);
}

static bool firstRun = true;

void HOL::OpenXR::HandTracking::updateGestures()
{
	// printf("################\n");

	HOL::GestureData data;
	data.aimState[0] = &this->mLeftHand.mAimState;
	data.aimState[1] = &this->mRightHand.mAimState;
	data.joints[0] = this->mLeftHand.getLastJointLocations();
	data.joints[1] = this->mRightHand.getLastJointLocations();
	// TODO: HMD pose

	for (auto& action : this->mActions)
	{
		action->evaluate(data);
	}
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

void HOL::OpenXR::HandTracking::drawHands()
{
	auto colorGrey = IM_COL32(155, 155, 155, 255);
	auto colorWhite = IM_COL32(255, 255, 255, 255);
	auto vis = HOL::UserInterface::Current->getVisualizer();

	for (int i = 0; i < HandSide::HandSide_MAX; i++)
	{
		XrHandJointLocationEXT* jointLocations = this->getHand((HandSide)i).getLastJointLocations();

		for (int j = 0; j < XR_HAND_JOINT_COUNT_EXT; j++)
		{
			XrHandJointLocationEXT& joint = jointLocations[j];

			// WIll replace this later anyway so nevermind wasteful conversion
			vis->submitPoint(OpenXR::toEigenVector(joint.pose.position), colorGrey, 5);
		}

		// Also draw some white skeleton lines
		{
			for (int finger = 0; finger < FingerType_MAX; finger++)
			{
				XrHandJointEXT rootJoint = OpenXR::getRootJoint((FingerType)finger);
				for (int j = 0; j < 4; j++)
				{
					XrHandJointLocationEXT& joint = jointLocations[rootJoint + j];
					XrHandJointLocationEXT& nextJoint = jointLocations[rootJoint + j + 1];
					vis->submitLine(OpenXR::toEigenVector(joint.pose.position),
									OpenXR::toEigenVector(nextJoint.pose.position),
									colorWhite,
									2);
				}
			}
		}

		XrHandJointLocationEXT& palm = jointLocations[XR_HAND_JOINT_PALM_EXT];

		// This doesn't super go here but it's a good place for it.
		if (i == HandSide::LeftHand && HOL::Config.visualizer.followLeftHand)
		{
			vis->centerTo(OpenXR::toEigenVector(palm.pose.position));
		}
		else if (i == HandSide::RightHand && HOL::Config.visualizer.followRightHand)
		{
			vis->centerTo(OpenXR::toEigenVector(palm.pose.position));
		}
	}
}
