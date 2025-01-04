#include "HandTracking.h"
#include "HandTrackingInterface.h"
#include <HandOfLesserCommon.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "src/core/ui/user_interface.h"
#include "XrUtils.h"
#include "src/core/settings_global.h"
#include "xr_hand_utils.h"

#include "src/hands/input/settings_toggle_input.h"
#include "src/hands/action/button_action.h"
#include "src/hands/gesture/chain_gesture.h"
#include "src/hands/gesture/combo_gesture.h"
#include "src/hands/gesture/finger_curl_gesture.h"
#include "src/hands/action/trigger_action.h"
#include "src/hands/input/steamvr_float_input.h"
#include "src/hands/input/steamvr_bool_input.h"
#include "src/steamvr/input_wrapper.h"
#include "src/steamvr/steamvr_input.h"

using namespace HOL;
using namespace HOL::OpenXR;
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
	// Joystick
	{
		HandSide side = HandSide::LeftHand;

		auto handDragAction = HandDragAction::Create()->setup(side, XrHandJointEXT::XR_HAND_JOINT_THUMB_TIP_EXT);

		auto triggerGesture = OpenHandPinchGesture::Gesture::Create();
		triggerGesture->parameters.pinchFinger = FingerType::FingerMiddle;
		triggerGesture->parameters.side = side;
		triggerGesture->setup();


		auto holdGesture = ProximityGesture::Create();
		holdGesture->setup(FingerType::FingerMiddle, side);

		ActionParameters actionParams = handDragAction->getParameters();
		actionParams.minReleaseTime = 0ms;
		actionParams.releaseThreshold = 0.8;

		handDragAction->setTriggerGesture(triggerGesture);
		handDragAction->setHoldGesture(holdGesture);
		handDragAction->addSink(
			InputType::XAxis,
			SteamVRFloatInput::Create()->setup(side, SteamVR::Input::Joystick.x()));
		handDragAction->addSink(
			InputType::ZAxis,
			SteamVRFloatInput::Create()->setup(side, SteamVR::Input::Joystick.y()));
		handDragAction->addSink(
			InputType::Touch,
			SteamVRBoolInput::Create()->setup(side, SteamVR::Input::Joystick.touch()));

		this->mActions.push_back(handDragAction);
	}

	// Toggle input
	{
		HandSide side = HandSide::RightHand;

		auto chainGesture = ChainGesture::Gesture::Create();

		std::vector<HOL::FingerType> allPinchFingers = {FingerType::FingerIndex,
														FingerType::FingerMiddle,
														FingerType::FingerRing,
														FingerType::FingerLittle};
		std::reverse(allPinchFingers.begin(),
					 allPinchFingers.end()); // Let's do from pinky to index

		for (auto otherFinger : allPinchFingers)
		{
			auto triggerGesture = OpenHandPinchGesture::Gesture::Create();
			triggerGesture->parameters.pinchFinger = otherFinger;
			triggerGesture->parameters.side = side;
			triggerGesture->setup();

			chainGesture->addGesture(triggerGesture);
		}

		auto settingsToggleInput = SettingsToggleInput::Create()->setup(HolSetting::SendSteamVRInput);

		auto buttonAction = ButtonAction::Create();
		buttonAction->setTriggerGesture(chainGesture);
		buttonAction->addSink(InputType::Button, settingsToggleInput);

		this->mActions.push_back(buttonAction);
	}


	// Y ( vrchat menu button )
	{
		HandSide side = HandSide::LeftHand;

		auto chainGesture = ChainGesture::Gesture::Create();

		std::vector<HOL::FingerType> allPinchFingers = {FingerType::FingerIndex,
														FingerType::FingerMiddle,
														FingerType::FingerRing,
														FingerType::FingerLittle};
		std::reverse(allPinchFingers.begin(),
					 allPinchFingers.end()); // Let's do from pinky to index

		for (auto otherFinger : allPinchFingers)
		{
			auto triggerGesture = OpenHandPinchGesture::Gesture::Create();
			triggerGesture->parameters.pinchFinger = otherFinger;
			triggerGesture->parameters.side = side;
			triggerGesture->setup();

			chainGesture->addGesture(triggerGesture);
		}

		auto buttonAction = ButtonAction::Create();
		buttonAction->setTriggerGesture(chainGesture);

		buttonAction.get()->addSink(
			InputType::Button, 
			SteamVRBoolInput::Create()->setup(side, SteamVR::Input::Y.click()));

		this->mActions.push_back(buttonAction);
	}

	// X ( vrchat mute button )
	{
		HandSide side = HandSide::LeftHand;

		auto chainGesture = ChainGesture::Gesture::Create();

		// Index to pinky
		std::vector<HOL::FingerType> allPinchFingers = {FingerType::FingerIndex,
														FingerType::FingerMiddle,
														FingerType::FingerRing,
														FingerType::FingerLittle};

		for (auto otherFinger : allPinchFingers)
		{
			auto triggerGesture = OpenHandPinchGesture::Gesture::Create();
			triggerGesture->parameters.pinchFinger = otherFinger;
			triggerGesture->parameters.side = side;
			triggerGesture->setup();

			chainGesture->addGesture(triggerGesture);
		}

		auto buttonAction = ButtonAction::Create();
		buttonAction->setTriggerGesture(chainGesture);

		buttonAction.get()->addSink(
			InputType::Button, SteamVRBoolInput::Create()->setup(side, SteamVR::Input::X.click()));

		this->mActions.push_back(buttonAction);
	}

	// Grab

	{
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{

			auto grabGesture = ComboGesture::Gesture::Create();
			grabGesture->parameters.holdUntilAllReleased = true;

			std::vector<HOL::FingerType> allGrabFingers
				= {FingerType::FingerMiddle, FingerType::FingerRing, FingerType::FingerLittle};

			// Grab if all but index curled
			for (auto finger : allGrabFingers)
			{
				auto curlGesture = FingerCurlGesture::Gesture::Create();
				curlGesture->parameters.finger = finger;
				curlGesture->parameters.side = (HandSide)i;

				grabGesture->addGesture(curlGesture);
			}

			auto grabAction = TriggerAction::Create();
			grabAction->getParameters().releaseThreshold = 0.8;
			grabAction.get()->setTriggerGesture(grabGesture);
			grabAction.get()->addSink(
				InputType::Button,
				SteamVRBoolInput::Create()->setup((HandSide)i, SteamVR::Input::Grip.click()));
			grabAction.get()->addSink(
				InputType::Button,	// Using button instead of sink because quest controller has no click action
				SteamVRFloatInput::Create()->setup((HandSide)i, SteamVR::Input::Grip.value()));


			this->mActions.push_back(grabAction);
		}
	}

	// Trigger

	{
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{
			HandSide side = (HandSide)i;
		

			// Trigger is grab + pinch ( but temporarily not )
			auto grabPinchGesture = ComboGesture::Gesture::Create();
			grabPinchGesture->parameters.holdUntilAllReleased = false;

			std::vector<HOL::FingerType> allGrabFingers
				= {FingerType::FingerMiddle, FingerType::FingerRing, FingerType::FingerLittle};

			/*
			// Grab is all but index curled
			for (auto finger : allGrabFingers)
			{
				auto curlGesture = FingerCurlGesture::Create();
				curlGesture.get()->setup(finger, (HandSide)i);
				grabPinchGesture->addGesture(curlGesture);
			}*/

			// Use aim for pinch
			auto triggerGesture = ProximityGesture::Create();
			triggerGesture->setup(FingerType::FingerIndex, (HandSide)i);
			grabPinchGesture.get()->addGesture(triggerGesture);

			auto triggerAction = TriggerAction::Create();
			triggerAction.get()->setTriggerGesture(grabPinchGesture);

			triggerAction.get()->addSink(
				InputType::Button,
				SteamVRBoolInput::Create()->setup((HandSide)i, SteamVR::Input::Trigger.click()));
			triggerAction.get()->addSink(
				InputType::Button,
				SteamVRFloatInput::Create()->setup((HandSide)i, SteamVR::Input::Trigger.value()));

			this->mActions.push_back(triggerAction);
		}
	}
}

void HandTracking::updateHands(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mLeftHand.updateJointLocations(space, time);
	this->mRightHand.updateJointLocations(space, time);
}

void HandTracking::updateInputs()
{
	updateGestures();
}

static bool firstRun = true;

void HOL::OpenXR::HandTracking::updateGestures()
{
	// printf("################\n");

	HOL::Gesture::GestureData data;
	for (int i = 0; i < HandSide::HandSide_MAX; i++)
	{
		OpenXRHand& hand = getHand((HandSide)i);

		data.handPose[i] = &hand.handPose;
		data.joints[i] = hand.getLastJointLocations();
	}

	// TODO: HMD pose

	for (auto& action : this->mActions)
	{
		action->evaluate(data);
	}

	// float combo = this->mComboGesture.get()->evaluate(data);
	// printf("Combo: %.3f\n", combo);
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
