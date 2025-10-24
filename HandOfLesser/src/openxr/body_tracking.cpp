#include "body_tracking.h"
#include "src/core/ui/user_interface.h"
#include "XrUtils.h"
#include <src/core/settings_global.h>

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

	// Visualize palm orientation axes if enabled
	if (Config.visualizer.showBodyTrackingPalmAxes)
	{
		// Left palm
		auto& leftPalm = jointLocations[XR_BODY_JOINT_LEFT_HAND_PALM_FB];
		if (leftPalm.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
		{
			vis->submitOrientationAxes(OpenXR::toEigenVector(leftPalm.pose.position),
									   OpenXR::toEigenQuaternion(leftPalm.pose.orientation),
									   0.120f,
									   6.0f);
		}

		// Right palm
		auto& rightPalm = jointLocations[XR_BODY_JOINT_RIGHT_HAND_PALM_FB];
		if (rightPalm.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
		{
			vis->submitOrientationAxes(OpenXR::toEigenVector(rightPalm.pose.position),
									   OpenXR::toEigenQuaternion(rightPalm.pose.orientation),
									   0.120f,
									   6.0f);
		}
	}
}

OpenXRBody& HOL::OpenXR::BodyTracking::getBodyTracker()
{
	return mBodyTracker;
}

HOL::MultimodalPosePacket HOL::OpenXR::BodyTracking::getMultimodalPosePacket()
{
	MultimodalPosePacket packet;

	auto* bodyJoints = mBodyTracker.getLastJointLocations();

	/////////////
	// Offsets
	/////////////

	// We have a base offset for each controller type, which for the time being
	// has been configured to match what VD handtracking gives you.
	// The user-configurable offset is applied in addition to this.
	auto controllerOffset = HOL::getControllerBaseOffset(Config.handPose.controllerType);

	// Matches to controller position, matching what VD does
	Eigen::Vector3f controllerRotationOffset = controllerOffset.orientation;
	Eigen::Vector3f controllerTranslationOffset = controllerOffset.position;

	// Left hand palm
	auto& leftPalm = bodyJoints[XR_BODY_JOINT_LEFT_HAND_PALM_FB];
	{
		Eigen::Vector3f position = OpenXR::toEigenVector(leftPalm.pose.position);
		Eigen::Quaternionf rotation = OpenXR::toEigenQuaternion(leftPalm.pose.orientation);
		/*
		position = HOL::translateLocal(position, rotation, controllerTranslationOffset);
		rotation = HOL::rotateLocal(
			rotation, HOL::quaternionFromEulerAnglesDegrees(controllerRotationOffset));
		*/
		packet.leftHandPose.position = position;
		packet.leftHandPose.orientation = rotation;
		packet.leftHandTracked = leftPalm.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
	}

	// Flip for right hand
	controllerRotationOffset = flipHandRotation(controllerRotationOffset);
	controllerTranslationOffset = flipHandTranslation(controllerTranslationOffset);

	// Right hand palm
	auto& rightPalm = bodyJoints[XR_BODY_JOINT_RIGHT_HAND_PALM_FB];
	{
		Eigen::Vector3f position = OpenXR::toEigenVector(rightPalm.pose.position);
		Eigen::Quaternionf rotation = OpenXR::toEigenQuaternion(rightPalm.pose.orientation);
		/*
		position = HOL::translateLocal(position, rotation, controllerTranslationOffset);
		rotation = HOL::rotateLocal(
			rotation, HOL::quaternionFromEulerAnglesDegrees(controllerRotationOffset));
		*/

		packet.rightHandPose.position = position;
		packet.rightHandPose.orientation = rotation;
		packet.rightHandTracked = rightPalm.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
	}

	return packet;
}
