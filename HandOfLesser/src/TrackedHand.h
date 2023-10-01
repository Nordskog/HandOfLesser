#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <HandOfLesserCommon.h>
#include "simple_gesture.h"

using namespace HOL;

class TrackedHand
{
	public:
		TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side);
		void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);

		HOL::HandTransformPacket getTransformPacket();
		HOL::ControllerInputPacket getInputPacket();

		XrHandJointLocationEXT mJointocations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointVelocityEXT mJointVelocities[XR_HAND_JOINT_COUNT_EXT];
		XrHandTrackingAimStateFB mAimState{ XR_TYPE_HAND_TRACKING_AIM_STATE_FB };
		XrBool32 mPoseValid;
		XrHandJointLocationEXT mLocation;
		XrHandJointVelocityEXT mVelocity;

		SimpleGesture::SimpleGestureState mSimpleGestures[SimpleGesture::SimpleGestureType::SIMPLE_GESTURE_MAX];

	private:
		void init(xr::UniqueDynamicSession& session, XrHandEXT side);
		XrHandEXT mSide;
		XrHandTrackerEXT mHandTracker;
		XrPath mInputSourcePath;


};
