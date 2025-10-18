#include "body_tracking.h"
#include "src/core/ui/user_interface.h"
#include "XrUtils.h"

void HOL::OpenXR::BodyTracking::init(xr::UniqueDynamicInstance& instance,
									 xr::UniqueDynamicSession& session)
{
	this->mBodyTracker.init(session);
}

void HOL::OpenXR::BodyTracking::updateBody(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mBodyTracker.updateJointLocations(space, time);
}

void HOL::OpenXR::BodyTracking::drawBody()
{
	auto colorTracked = IM_COL32(0, 255, 0, 150);	 // Green semi-transparent for tracked
	auto colorNotTracked = IM_COL32(255, 0, 0, 150); // Red semi-transparent for not tracked

	auto vis = HOL::UserInterface::Current->getVisualizer();

	XrBodyJointLocationFB* jointLocations = this->mBodyTracker.getLastJointLocations();

	for (int j = 0; j < XR_BODY_JOINT_COUNT_FB; j++)
	{
		XrBodyJointLocationFB& joint = jointLocations[j];

		// Check if joint is tracked
		bool isTracked = (joint.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
						 == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

		auto color = isTracked ? colorTracked : colorNotTracked;

		// WIll replace this later anyway so nevermind wasteful conversion
		vis->submitPoint(OpenXR::toEigenVector(joint.pose.position), color, 7);
	}
}

OpenXRBody& HOL::OpenXR::BodyTracking::getBodyTracker()
{
	return mBodyTracker;
}

HOL::BodyTrackingHandPosePacket
HOL::OpenXR::BodyTracking::getBodyTrackingHandPosePacket(bool isOVR, bool isMultimodalEnabled)
{
	BodyTrackingHandPosePacket packet;
	packet.isOVR = isOVR;
	packet.isMultimodalEnabled = isMultimodalEnabled;

	auto* bodyJoints = mBodyTracker.getLastJointLocations();

	// Left hand wrist (body tracking index 19)
	auto& leftWrist = bodyJoints[XR_BODY_JOINT_LEFT_HAND_WRIST_FB];
	if (leftWrist.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
	{
		packet.leftHandPose.position = OpenXR::toEigenVector(leftWrist.pose.position);
		packet.leftHandPose.orientation = OpenXR::toEigenQuaternion(leftWrist.pose.orientation);
		packet.leftHandValid = true;
	}

	// Right hand wrist (body tracking index 45)
	auto& rightWrist = bodyJoints[XR_BODY_JOINT_RIGHT_HAND_WRIST_FB];
	if (rightWrist.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
	{
		packet.rightHandPose.position = OpenXR::toEigenVector(rightWrist.pose.position);
		packet.rightHandPose.orientation = OpenXR::toEigenQuaternion(rightWrist.pose.orientation);
		packet.rightHandValid = true;
	}

	return packet;
}
