#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

class HandTrackingInterface
{
public:
	static void init(xr::UniqueDynamicInstance& instance);

	// Hand
	static void createHandTracker(xr::UniqueDynamicSession& session,
								  XrHandEXT side,
								  XrHandTrackerEXT& handTrackerOut);
	static bool locateHandJoints(XrHandTrackerEXT& handTracker,
								 xr::UniqueDynamicSpace& space,
								 XrTime time,
								 XrHandJointLocationEXT* handJointLocationsOut,
								 XrHandJointVelocityEXT* handJointVelocitiesOut);
	static void destroyHandTracker();

	// Body
	static void createBodyTracker(xr::UniqueDynamicSession& session,
								  XrBodyTrackerFB& bodyTrackerOut);
	static void destroyBodyTracker();
	static float locateBodyJoints(XrBodyTrackerFB& bodyTracker,
								  xr::UniqueDynamicSpace& space,
								  XrTime time,
								  XrBodyJointLocationFB* bodyJointLocationsOut);

private:
	static PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT_;
	static PFN_xrDestroyHandTrackerEXT xrDestroyHandTrackerEXT_;
	static PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT_;

	static PFN_xrCreateBodyTrackerFB xrCreateBodyTrackerFB_;
	static PFN_xrDestroyBodyTrackerFB xrDestroyBodyTrackerFB_;
	static PFN_xrLocateBodyJointsFB xrLocateBodyJointsFB_;

	static void initFunctions(xr::UniqueDynamicInstance& instance);
};