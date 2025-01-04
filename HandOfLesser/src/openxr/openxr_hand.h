#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <HandOfLesserCommon.h>
#include "src/hands/hand_pose.h"

using namespace HOL;

class OpenXRHand
{
public:
	void init(xr::UniqueDynamicSession& session, HOL::HandSide side);
	void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);

	HandPose handPose;
	XrHandJointLocationEXT* getLastJointLocations();

private:
	void calculateCurlSplay();

	HOL::HandSide mSide;
	XrHandTrackerEXT mHandTracker;
	XrPath mInputSourcePath;
	XrHandJointLocationEXT mJointLocations[XR_HAND_JOINT_COUNT_EXT];
	XrHandJointVelocityEXT mJointVelocities[XR_HAND_JOINT_COUNT_EXT];

	HOL::PoseLocation mPrevRawPose;
};
