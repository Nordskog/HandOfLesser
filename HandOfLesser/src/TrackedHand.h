#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <HandOfLesserCommon.h>

class TrackedHand
{
	public:
		TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side);
		void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);
		HOL::HandTransformPacket getNativePacket();
		XrHandJointLocationEXT mJointocations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointVelocityEXT mJointVelocities[XR_HAND_JOINT_COUNT_EXT];
		XrHandTrackingAimStateFB mAimState{ XR_TYPE_HAND_TRACKING_AIM_STATE_FB };
		XrBool32 mPoseValid;
		XrHandJointLocationEXT mLocation;
		XrHandJointVelocityEXT mVelocity;


	private:
		void init(xr::UniqueDynamicSession& session, XrHandEXT side);
		XrHandEXT mSide;
		XrHandTrackerEXT mHandTracker;


};
