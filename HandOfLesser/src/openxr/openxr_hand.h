#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <HandOfLesserCommon.h>
#include "src/hands/simple_gesture.h"
#include "src/hands/hand_pose.h"

using namespace HOL;

class OpenXRHand
{
public:
	OpenXRHand(xr::UniqueDynamicSession& session, HOL::HandSide side);
	void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);

	HandPose handPose;
	SimpleGesture::SimpleGestureState
		simpleGestures[SimpleGesture::SimpleGestureType::SIMPLE_GESTURE_MAX];
	XrHandTrackingAimStateFB mAimState{XR_TYPE_HAND_TRACKING_AIM_STATE_FB};

private:
	void init(xr::UniqueDynamicSession& session, HOL::HandSide side);
	void calculateCurlSplay();

	HOL::HandSide mSide;
	XrHandTrackerEXT mHandTracker;
	XrPath mInputSourcePath;
	XrHandJointLocationEXT mJointLocations[XR_HAND_JOINT_COUNT_EXT];
	XrHandJointVelocityEXT mJointVelocities[XR_HAND_JOINT_COUNT_EXT];
};
