#include "openxr_body.h"
#include "HandTrackingInterface.h"

#include "XrUtils.h"

#include "src/core/ui/display_global.h"

#include <iostream>

void OpenXRBody::init(xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::createBodyTracker(session, mBodyTracker);
}

void OpenXRBody::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	if (mBodyTracker == nullptr)
		return;

	std::copy(std::begin(mJointLocations),
			  std::end(mJointLocations),
			  std::begin(mPreviousJointLocations));



	this->confidence = HandTrackingInterface::locateBodyJoints(this->mBodyTracker, space, time, this->mJointLocations);
	this->active = confidence > 0.0f;

	// Generate our own palm joints from body tracking.
	// The joint is missing in the oculus runtime, and calculated incorrectly in VDXR.
	generateMissingPalmJoint(HandSide::LeftHand);
	generateMissingPalmJoint(HandSide::RightHand);

	// Terrible but good enough for now
	auto& torsoJoint = this->mJointLocations[XR_BODY_JOINT_CHEST_FB];
	auto& prevTorsoJoint = this->mPreviousJointLocations[XR_BODY_JOINT_CHEST_FB];

	auto& wristJointLeft = this->mJointLocations[XR_BODY_JOINT_LEFT_HAND_WRIST_FB];
	auto& prevWristJointLeft = this->mPreviousJointLocations[XR_BODY_JOINT_LEFT_HAND_WRIST_FB];

	auto& wristJointRight = this->mJointLocations[XR_BODY_JOINT_RIGHT_HAND_WRIST_FB];
	auto& prevWristJointRight = this->mPreviousJointLocations[XR_BODY_JOINT_RIGHT_HAND_WRIST_FB];

	// VDXR workaround
	{
		// Ensure data has been updated, torso should always change
		if (torsoJoint.pose.position.x != prevTorsoJoint.pose.position.x
			&& torsoJoint.pose.position.y != prevTorsoJoint.pose.position.y)
		{
			// VD freezes values starting from wrist when hand tracking is lost.
			// We base controller hand/controller position on palm position last wrist transform
			// and current wrist twist bone transform to calculate where it should be.
			if (wristJointLeft.pose.position.x == prevWristJointLeft.pose.position.x
				&& wristJointLeft.pose.position.y == prevWristJointLeft.pose.position.y)
			{
				// std::printf("Wrist is GONE!!\n");
				generatePalmPosition(HandSide::LeftHand);
			}
			else
			{
				// Only ones we care about.
				mLastWithTrackedHands[XR_BODY_JOINT_LEFT_HAND_PALM_FB]
					= mJointLocations[XR_BODY_JOINT_LEFT_HAND_PALM_FB];
				mLastWithTrackedHands[XR_BODY_JOINT_LEFT_ARM_LOWER_FB]
					= mJointLocations[XR_BODY_JOINT_LEFT_ARM_LOWER_FB];
			}

			if (wristJointRight.pose.position.x == prevWristJointRight.pose.position.x
				&& wristJointRight.pose.position.y == prevWristJointRight.pose.position.y)
			{
				// std::printf("Wrist is GONE!!\n");
				generatePalmPosition(HandSide::RightHand);
			}
			else
			{
				// Only ones we care about.
				mLastWithTrackedHands[XR_BODY_JOINT_RIGHT_HAND_PALM_FB]
					= mJointLocations[XR_BODY_JOINT_RIGHT_HAND_PALM_FB];
				mLastWithTrackedHands[XR_BODY_JOINT_RIGHT_ARM_LOWER_FB]
					= mJointLocations[XR_BODY_JOINT_RIGHT_ARM_LOWER_FB];
			}
		}
	}

	// Display
	HOL::display::BodyTracking.confidence = this->confidence;
}

XrBodyJointLocationFB* OpenXRBody::getLastJointLocations()
{
	return this->mJointLocations;
}

XrBodyTrackerFB OpenXRBody::getBodyTrackerFB()
{
	return this->mBodyTracker;
}

void OpenXRBody::generatePalmPosition(HandSide side)
{
	XrBodyJointLocationFB& lastTwist
		= this->mLastWithTrackedHands[side == HandSide::LeftHand
										  ? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
																 : XR_BODY_JOINT_RIGHT_ARM_LOWER_FB];
	XrBodyJointLocationFB& lastPalm
		= this->mLastWithTrackedHands[side == HandSide::LeftHand
										  ? XR_BODY_JOINT_LEFT_HAND_PALM_FB
										  : XR_BODY_JOINT_RIGHT_HAND_PALM_FB];

	XrBodyJointLocationFB& currentTwist
		= this->mJointLocations[side == HandSide::LeftHand
									? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
														   : XR_BODY_JOINT_RIGHT_ARM_LOWER_FB];

	XrBodyJointLocationFB& currentPalm
		= this->mJointLocations[side == HandSide::LeftHand
											? XR_BODY_JOINT_LEFT_HAND_PALM_FB
											: XR_BODY_JOINT_RIGHT_HAND_PALM_FB];

	Eigen::Vector3f relativePos;
	Eigen::Quaternionf relativeRot;

	calculateRelativeTransform(lastTwist, lastPalm, relativePos, relativeRot);
	applyRelativeTransform(currentTwist, currentPalm, relativePos, relativeRot);

}


void OpenXRBody::calculateRelativeTransform(const XrBodyJointLocationFB& baseJoint,
											const XrBodyJointLocationFB& childJoint,
											Eigen::Vector3f& relativePos,
											Eigen::Quaternionf& relativeRot)
{
	auto basePos = OpenXR::toEigenVector(baseJoint.pose.position);
	auto baseRot = OpenXR::toEigenQuaternion(baseJoint.pose.orientation);

	auto childPos = OpenXR::toEigenVector(childJoint.pose.position);
	auto childRot = OpenXR::toEigenQuaternion(childJoint.pose.orientation);


	relativePos = baseRot.inverse() * (childPos - basePos);
	relativeRot = baseRot.inverse() * childRot;
}

void OpenXRBody::applyRelativeTransform(const XrBodyJointLocationFB& baseJoint,
										XrBodyJointLocationFB& childJoint,
										const Eigen::Vector3f& relativePos,
										const Eigen::Quaternionf& relativeRot)
{
	auto basePos = OpenXR::toEigenVector(baseJoint.pose.position);
	auto baseRot = OpenXR::toEigenQuaternion(baseJoint.pose.orientation);


	Eigen::Vector3f childPos = basePos + baseRot * relativePos;
	Eigen::Quaternionf childRot = baseRot * relativeRot;

	childJoint.pose.position.x = childPos.x();
	childJoint.pose.position.y = childPos.y();
	childJoint.pose.position.z = childPos.z();

	childJoint.pose.orientation.w = childRot.w();
	childJoint.pose.orientation.x = childRot.x();
	childJoint.pose.orientation.y = childRot.y();
	childJoint.pose.orientation.z = childRot.z();
}

void OpenXRBody::generateMissingPalmJoint(HandSide side)
{
	// Body tracking on Oculus does not provide palm joint, so we generate it
	// according to OpenXR spec: center of middle finger's metacarpal bone

	// Get the joint indices for this hand
	XrBodyJointFB metacarpalJoint = (side == HandSide::LeftHand)
										? XR_BODY_JOINT_LEFT_HAND_MIDDLE_METACARPAL_FB
										: XR_BODY_JOINT_RIGHT_HAND_MIDDLE_METACARPAL_FB;

	XrBodyJointFB proximalJoint = (side == HandSide::LeftHand)
									  ? XR_BODY_JOINT_LEFT_HAND_MIDDLE_PROXIMAL_FB
									  : XR_BODY_JOINT_RIGHT_HAND_MIDDLE_PROXIMAL_FB;

	XrBodyJointFB palmJoint
		= (side == HandSide::LeftHand) ? XR_BODY_JOINT_LEFT_HAND_PALM_FB : XR_BODY_JOINT_RIGHT_HAND_PALM_FB;

	// Check if joints are valid
	auto& metacarpal = mJointLocations[metacarpalJoint];
	auto& proximal = mJointLocations[proximalJoint];

	if (!(metacarpal.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		|| !(proximal.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT))
	{
		return; // Can't generate palm without valid metacarpal joints
	}

	// Calculate position: midpoint of metacarpal bone
	Eigen::Vector3f metacarpalPos = OpenXR::toEigenVector(metacarpal.pose.position);
	Eigen::Vector3f proximalPos = OpenXR::toEigenVector(proximal.pose.position);
	Eigen::Vector3f palmPos = (metacarpalPos + proximalPos) * 0.5f;

	// Calculate orientation according to OpenXR spec:
	// +Z points away from fingertips (along metacarpal towards wrist)
	// +Y points towards back of hand (perpendicular to palm surface)
	// +X follows right-hand rule

	Eigen::Vector3f zAxis = (metacarpalPos - proximalPos).normalized(); // Points from finger to wrist

	// Use metacarpal orientation's Y axis as reference for hand back direction
	Eigen::Quaternionf metacarpalRot = OpenXR::toEigenQuaternion(metacarpal.pose.orientation);
	Eigen::Vector3f metacarpalY = metacarpalRot * Eigen::Vector3f(0, 1, 0);

	// Calculate Y axis (perpendicular to palm, pointing to back of hand)
	Eigen::Vector3f xAxis = metacarpalY.cross(zAxis).normalized();
	Eigen::Vector3f yAxis = zAxis.cross(xAxis).normalized();

	// Construct rotation matrix from axes
	Eigen::Matrix3f rotMatrix;
	rotMatrix.col(0) = xAxis;
	rotMatrix.col(1) = yAxis;
	rotMatrix.col(2) = zAxis;

	Eigen::Quaternionf palmRot(rotMatrix);

	// Set palm joint location
	auto& palm = mJointLocations[palmJoint];
	palm.pose.position.x = palmPos.x();
	palm.pose.position.y = palmPos.y();
	palm.pose.position.z = palmPos.z();

	palm.pose.orientation.w = palmRot.w();
	palm.pose.orientation.x = palmRot.x();
	palm.pose.orientation.y = palmRot.y();
	palm.pose.orientation.z = palmRot.z();

	// Mark as valid and tracked
	palm.locationFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT
						 | XR_SPACE_LOCATION_POSITION_TRACKED_BIT
						 | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
}

