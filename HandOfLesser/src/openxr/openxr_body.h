#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <HandOfLesserCommon.h>

using namespace HOL;

class OpenXRBody
{
public:
	void init(xr::UniqueDynamicSession& session);
	void updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time);

	XrBodyJointLocationFB* getLastJointLocations();

private:

	XrBodyTrackerFB mBodyTracker;
	XrPath mInputSourcePath;
	XrBodyJointLocationFB mJointLocations[XR_BODY_JOINT_COUNT_FB];
};
