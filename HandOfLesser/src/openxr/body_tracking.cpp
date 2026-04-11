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
	auto colorUntracked = IM_COL32(255, 180, 0, 170); // Orange for valid but untracked
	auto colorInvalid = IM_COL32(255, 0, 0, 150);	 // Red for invalid joints
	auto colorFallbackPalm = IM_COL32(255, 255, 0, 255);

	auto vis = HOL::UserInterface::Current->getVisualizer();
	if (!vis->isActive())
	{
		return;
	}

	XrBodyJointLocationFB* jointLocations = this->mBodyTracker.getLastJointLocations();
	if (jointLocations == nullptr)
	{
		return;
	}

	for (int j = 0; j < XR_BODY_JOINT_COUNT_FB; j++)
	{
		XrBodyJointLocationFB& joint = jointLocations[j];

		bool isValid = (joint.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
					   == XR_SPACE_LOCATION_POSITION_VALID_BIT;
		// Check if joint is tracked
		bool isTracked = (joint.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
						 == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

		auto color = isValid ? (isTracked ? colorTracked : colorUntracked) : colorInvalid;

		vis->submitPoint(OpenXR::toEigenVector(joint.pose.position), color, 7);
	}

	const auto drawFallbackPalm = [&](XrBodyJointFB jointIndex)
	{
		auto& palm = jointLocations[jointIndex];
		bool isValid = (palm.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
					   == XR_SPACE_LOCATION_POSITION_VALID_BIT;
		bool isTracked = (palm.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
						 == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
		if (isValid && !isTracked)
		{
			vis->submitPoint(OpenXR::toEigenVector(palm.pose.position), colorFallbackPalm, 7);
		}
	};

	drawFallbackPalm(XR_BODY_JOINT_LEFT_HAND_PALM_FB);
	drawFallbackPalm(XR_BODY_JOINT_RIGHT_HAND_PALM_FB);

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

HOL::MultimodalPosePayload HOL::OpenXR::BodyTracking::getMultimodalPosePayload()
{
	MultimodalPosePayload payload;

	auto* bodyJoints = mBodyTracker.getLastJointLocations();
	if (bodyJoints == nullptr)
	{
		return payload;
	}

	/////////////
	// Offsets
	/////////////

	// We have a base offset configured to match what VDXR handtracking gives you.
	// The user-configurable offset is applied in addition to this.
	auto controllerOffset = HOL::getControllerBaseOffset();

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
		payload.leftHandPose.position = position;
		payload.leftHandPose.orientation = rotation;
		payload.leftHandTracked = leftPalm.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
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

		payload.rightHandPose.position = position;
		payload.rightHandPose.orientation = rotation;
		payload.rightHandTracked = rightPalm.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
	}

	return payload;
}

std::vector<HOL::BodyTrackerPosePayload> HOL::OpenXR::BodyTracking::getBodyTrackerPayloads()
{
	std::vector<HOL::BodyTrackerPosePayload> payloads;

	// Get body tracking joint data
	XrBodyJointLocationFB* jointLocations = mBodyTracker.getLastJointLocations();

	if (!jointLocations)
		return payloads;

	// Send payload for each enabled tracker
	for (int i = 0; i < static_cast<int>(BodyTrackerRole::TrackerRole_MAX); i++)
	{
		BodyTrackerRole role = static_cast<BodyTrackerRole>(i);

		// Check if this tracker is enabled
		bool isEnabled = false;
		switch (role)
		{
			case BodyTrackerRole::Hips:
				isEnabled = Config.bodyTrackers.enableHips;
				break;
			case BodyTrackerRole::Chest:
				isEnabled = Config.bodyTrackers.enableChest;
				break;
			case BodyTrackerRole::LeftUpperArm:
				isEnabled = Config.bodyTrackers.enableLeftUpperArm;
				break;
			case BodyTrackerRole::LeftLowerArm:
				isEnabled = Config.bodyTrackers.enableLeftLowerArm;
				break;
			case BodyTrackerRole::RightUpperArm:
				isEnabled = Config.bodyTrackers.enableRightUpperArm;
				break;
			case BodyTrackerRole::RightLowerArm:
				isEnabled = Config.bodyTrackers.enableRightLowerArm;
				break;
			default:
				isEnabled = false;
				break;
		}

		if (!isEnabled)
			continue;

		// Get the joint for this tracker role
		XrBodyJointFB joint = bodyTrackerRoleToJoint(role);
		auto& location = jointLocations[joint];

		// Calculate tracker position (may be adjusted from joint position)
		Eigen::Vector3f trackerPosition;

		// Adjust position based on tracker role to match physical tracker mounting positions
		switch (role)
		{
			case BodyTrackerRole::LeftLowerArm:
			case BodyTrackerRole::RightLowerArm:
			{
				// Lower arm tracker: midpoint between elbow (lower arm joint) and wrist
				XrBodyJointFB wristJoint = (role == BodyTrackerRole::LeftLowerArm)
					? XR_BODY_JOINT_LEFT_HAND_WRIST_TWIST_FB
					: XR_BODY_JOINT_RIGHT_HAND_WRIST_TWIST_FB;
				auto& wristLocation = jointLocations[wristJoint];

				trackerPosition.x() = (location.pose.position.x + wristLocation.pose.position.x) * 0.5f;
				trackerPosition.y() = (location.pose.position.y + wristLocation.pose.position.y) * 0.5f;
				trackerPosition.z() = (location.pose.position.z + wristLocation.pose.position.z) * 0.5f;
				break;
			}
			case BodyTrackerRole::LeftUpperArm:
			case BodyTrackerRole::RightUpperArm:
			{
				// Upper arm tracker: midpoint between shoulder (upper arm joint) and elbow (lower arm joint)
				XrBodyJointFB elbowJoint = (role == BodyTrackerRole::LeftUpperArm)
					? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
					: XR_BODY_JOINT_RIGHT_ARM_LOWER_FB;
				auto& elbowLocation = jointLocations[elbowJoint];

				trackerPosition.x() = (location.pose.position.x + elbowLocation.pose.position.x) * 0.5f;
				trackerPosition.y() = (location.pose.position.y + elbowLocation.pose.position.y) * 0.5f;
				trackerPosition.z() = (location.pose.position.z + elbowLocation.pose.position.z) * 0.5f;
				break;
			}
			default:
				// For other trackers (hips, chest), use the joint position directly
				trackerPosition.x() = location.pose.position.x;
				trackerPosition.y() = location.pose.position.y;
				trackerPosition.z() = location.pose.position.z;
				break;
		}

		// Create payload
		BodyTrackerPosePayload payload;
		payload.role = role;
		payload.active = mBodyTracker.active;
		payload.valid = (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
		payload.tracked = (location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) != 0;

		// Set adjusted position
		payload.location.position = trackerPosition;

		// Use orientation from the primary joint
		payload.location.orientation.w() = location.pose.orientation.w;
		payload.location.orientation.x() = location.pose.orientation.x;
		payload.location.orientation.y() = location.pose.orientation.y;
		payload.location.orientation.z() = location.pose.orientation.z;

		// Arm-mounted trackers look visually upside down with the raw body-joint
		// orientation, so flip them 180 degrees around their local forward axis.
		if (role == BodyTrackerRole::RightUpperArm || role == BodyTrackerRole::RightLowerArm)
		{
			payload.location.orientation = HOL::rotateLocal(
				payload.location.orientation, HOL::quaternionFromEulerAnglesDegrees(0, 0, 180));
		}

		// Visualize body tracker axes if enabled
		if (Config.visualizer.showBodyTrackerAxes && payload.valid)
		{
			auto* vis = HOL::UserInterface::Current->getVisualizer();
			vis->submitOrientationAxes(trackerPosition,
									   payload.location.orientation,
									   0.060f,	// Half the size of palm axes (0.120f / 2)
									   3.0f);	// Half the line width (6.0f / 2)
		}

		// Velocities left at zero (body tracking velocity data is unreliable)
		payload.velocity.linearVelocity = Eigen::Vector3f::Zero();
		payload.velocity.angularVelocity = Eigen::Vector3f::Zero();

		payloads.push_back(payload);
	}

	return payloads;
}
