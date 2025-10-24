#include "openxr_body.h"
#include "HandTrackingInterface.h"

#include "XrUtils.h"

#include "src/core/ui/display_global.h"
#include "src/core/state_global.h"

#include <iostream>

// Oculus runtime returns bogus orientations for body tracking.
// They're not OVR-style either, no idea. Fix them as needed.
Eigen::Quaternionf OpenXRBody::fixOculusJointOrientation(const Eigen::Vector3f& currentJointPos,
														 const Eigen::Vector3f& nextJointPos,
														 const Eigen::Quaternionf& bogusOrientation)
{
	// -Z forward
	Eigen::Vector3f zAxis = -((nextJointPos - currentJointPos).normalized());

	// Existing Z is where X should be, roughly.
	Eigen::Vector3f xAxis = (bogusOrientation * Eigen::Vector3f::UnitZ()).normalized();

	// Y is their cross product
	Eigen::Vector3f yAxis = zAxis.cross(xAxis).normalized();

	Eigen::Matrix3f rotMatrix;
	rotMatrix.col(0) = xAxis;
	rotMatrix.col(1) = yAxis;
	rotMatrix.col(2) = zAxis;

	return Eigen::Quaternionf(rotMatrix);
}

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

	this->confidence = HandTrackingInterface::locateBodyJoints(
		this->mBodyTracker, space, time, this->mJointLocations);
	this->active = confidence > 0.0f;

	// Generate our own palm joints from body tracking.
	// The joint is missing in the oculus runtime, and calculated incorrectly in VDXR.
	generateMissingPalmJoint(HandSide::LeftHand);
	generateMissingPalmJoint(HandSide::RightHand);

	// Preserve wrist rotation when hand tracking is lost
	// Use metacarpal tracking state to detect loss - fingers remain tracked until fully lost
	// VDXR marks wrist onwards as tracked when tracked, rest untracked.
	// Oculus only marks metacarpal and onwards.
	{
		// Left hand
		auto& leftMetacarpal = mJointLocations[XR_BODY_JOINT_LEFT_HAND_MIDDLE_METACARPAL_FB];
		if (leftMetacarpal.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		{
			// Hand tracking is active - save current palm and forearm positions
			mLastWithTrackedHands[XR_BODY_JOINT_LEFT_HAND_PALM_FB]
				= mJointLocations[XR_BODY_JOINT_LEFT_HAND_PALM_FB];
			mLastWithTrackedHands[XR_BODY_JOINT_LEFT_ARM_LOWER_FB]
				= mJointLocations[XR_BODY_JOINT_LEFT_ARM_LOWER_FB];
		}
		else
		{
			// Hand tracking lost - preserve wrist rotation relative to forearm
			preserveWristRotation(HandSide::LeftHand);
		}

		// Right hand
		auto& rightMetacarpal = mJointLocations[XR_BODY_JOINT_RIGHT_HAND_MIDDLE_METACARPAL_FB];
		if (rightMetacarpal.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		{
			// Hand tracking is active - save current palm and forearm positions
			mLastWithTrackedHands[XR_BODY_JOINT_RIGHT_HAND_PALM_FB]
				= mJointLocations[XR_BODY_JOINT_RIGHT_HAND_PALM_FB];
			mLastWithTrackedHands[XR_BODY_JOINT_RIGHT_ARM_LOWER_FB]
				= mJointLocations[XR_BODY_JOINT_RIGHT_ARM_LOWER_FB];
		}
		else
		{
			// Hand tracking lost - preserve wrist rotation relative to forearm
			preserveWristRotation(HandSide::RightHand);
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

void OpenXRBody::preserveWristRotation(HandSide side)
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
		= this->mJointLocations[side == HandSide::LeftHand ? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
														   : XR_BODY_JOINT_RIGHT_ARM_LOWER_FB];

	XrBodyJointLocationFB& currentPalm
		= this->mJointLocations[side == HandSide::LeftHand ? XR_BODY_JOINT_LEFT_HAND_PALM_FB
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

	XrBodyJointFB palmJoint = (side == HandSide::LeftHand) ? XR_BODY_JOINT_LEFT_HAND_PALM_FB
														   : XR_BODY_JOINT_RIGHT_HAND_PALM_FB;

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

	Eigen::Vector3f zAxis
		= (metacarpalPos - proximalPos).normalized(); // Points from finger to wrist

	// Just reuse metacarpal for orientation
	Eigen::Quaternionf palmOrientation = OpenXR::toEigenQuaternion(metacarpal.pose.orientation);

	// OVR returns bad body tracking orientations
	if (HOL::state::Runtime.isOVR)
	{
		// But they may fix it some day, so check if existing orientation is within
		// 45 degrees of currentJoint->NextJoint orientation and only apply if it's off.
		Eigen::Vector3f expectedForward = (proximalPos - palmPos).normalized();
		Eigen::Vector3f currentForward = -(palmOrientation * Eigen::Vector3f::UnitZ());

		if (expectedForward.dot(currentForward) < 0.707f) // 45 degrees'ish
		{
			palmOrientation = fixOculusJointOrientation(palmPos, proximalPos, palmOrientation);
		}
	}

	// Set palm joint location
	auto& palm = mJointLocations[palmJoint];
	palm.pose.position.x = palmPos.x();
	palm.pose.position.y = palmPos.y();
	palm.pose.position.z = palmPos.z();

	palm.pose.orientation.w = palmOrientation.w();
	palm.pose.orientation.x = palmOrientation.x();
	palm.pose.orientation.y = palmOrientation.y();
	palm.pose.orientation.z = palmOrientation.z();

	// Inherit tracking state directly from metacarpal joint
	palm.locationFlags = metacarpal.locationFlags;
}
