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
	XrBodyTrackerFB getBodyTrackerFB();
	float confidence;
	bool active = false;

private:

	void calculateRelativeTransform(const XrBodyJointLocationFB& baseJoint,
									const XrBodyJointLocationFB& childJoint, Eigen::Vector3f& relativePos, Eigen::Quaternionf& relativeRot);
	void applyRelativeTransform(const XrBodyJointLocationFB& baseJoint,
								XrBodyJointLocationFB& childJoint,
								const Eigen::Vector3f& relativePos,
								const Eigen::Quaternionf& relativeRot);

	void generatePalmPosition(HandSide side);
	void generateMissingPalmJoint(HandSide side);

	XrBodyJointLocationFB mLastWithTrackedHands[XR_BODY_JOINT_COUNT_FB];


	XrBodyTrackerFB mBodyTracker;
	XrPath mInputSourcePath;
	XrBodyJointLocationFB mJointLocations[XR_BODY_JOINT_COUNT_FB];
	XrBodyJointLocationFB mPreviousJointLocations[XR_BODY_JOINT_COUNT_FB];
};
