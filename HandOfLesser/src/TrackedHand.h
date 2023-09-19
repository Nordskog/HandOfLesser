#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

class TrackedHand
{
	public:
		TrackedHand(xr::UniqueDynamicSession& session, XrHandEXT side);
		void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);


	private:
		void init(xr::UniqueDynamicSession& session, XrHandEXT side);
		XrHandTrackerEXT mHandTracker;
		XrHandJointLocationEXT mJointocations[XR_HAND_JOINT_COUNT_EXT];



};
