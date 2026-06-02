#include "openxr_body.h"
#include "HandTrackingInterface.h"

#include "XrUtils.h"

#include "src/core/ui/display_global.h"
#include "src/core/settings_global.h"
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

OpenXRBody::~OpenXRBody()
{
	this->shutdown();
}

void OpenXRBody::shutdown()
{
	HandTrackingInterface::destroyBodyTracker(this->mBodyTracker);
}

void OpenXRBody::init(xr::UniqueDynamicSession& session)
{
	if (!HOL::state::Runtime.supportsBodyTracking)
	{
		return;
	}

	HandTrackingInterface::createBodyTracker(session, mBodyTracker);
}

void OpenXRBody::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	if (mBodyTracker == nullptr)
		return;

	std::copy(std::begin(mJointLocations),
			  std::end(mJointLocations),
			  std::begin(mPreviousJointLocations));

	XrResult result = HandTrackingInterface::locateBodyJoints(
		this->mBodyTracker, space, time, this->mJointLocations, this->confidence);
	if (result != XR_SUCCESS)
	{
		this->confidence = 0.0f;
		this->active = false;
		HOL::display::BodyTracking.confidence = this->confidence;
		HOL::display::BodyTracking.headPoseValid = false;
		return;
	}

	this->active = confidence > 0.0f;

	// Generate our own palm joints from body tracking.
	// The joint is missing in the oculus runtime, and calculated incorrectly in VDXR.
	generateMissingPalmJoint(HandSide::LeftHand);
	generateMissingPalmJoint(HandSide::RightHand);

	// Preserve palm pose when hand tracking is lost.
	// Use metacarpal tracking state to detect loss - fingers remain tracked until fully lost
	// VDXR marks wrist onwards as tracked when tracked, rest untracked.
	// Oculus only marks metacarpal and onwards.
	{
		const bool useArmTrackingAnchor = canUseArmTrackingAnchor();

		// Left hand
		auto& leftMetacarpal = mJointLocations[XR_BODY_JOINT_LEFT_HAND_MIDDLE_METACARPAL_FB];
		if (leftMetacarpal.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		{
			updateTrackedPalmTransform(HandSide::LeftHand,
									 useArmTrackingAnchor ? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
														  : XR_BODY_JOINT_CHEST_FB,
									 time);
		}
		else
		{
			mHandTrackingAcquiredTime[(int)HandSide::LeftHand] = 0;
			mWasHandTrackingTracked[(int)HandSide::LeftHand] = false;
			preservePalmPose(HandSide::LeftHand);
		}

		// Right hand
		auto& rightMetacarpal = mJointLocations[XR_BODY_JOINT_RIGHT_HAND_MIDDLE_METACARPAL_FB];
		if (rightMetacarpal.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		{
			updateTrackedPalmTransform(HandSide::RightHand,
									 useArmTrackingAnchor ? XR_BODY_JOINT_RIGHT_ARM_LOWER_FB
														  : XR_BODY_JOINT_CHEST_FB,
									 time);
		}
		else
		{
			mHandTrackingAcquiredTime[(int)HandSide::RightHand] = 0;
			mWasHandTrackingTracked[(int)HandSide::RightHand] = false;
			preservePalmPose(HandSide::RightHand);
		}
	}

	// Buffer used by any callers so they don't get mid-correction data
	std::copy(std::begin(mJointLocations),
			  std::end(mJointLocations),
			  std::begin(mCorrectedJointLocations));

	HOL::display::BodyTracking.headPoseValid
		= this->getHeadPose(HOL::display::BodyTracking.headPose);

	// Display
	HOL::display::BodyTracking.confidence = this->confidence;
}

XrBodyJointLocationFB* OpenXRBody::getLastJointLocations()
{
	if (mBodyTracker == nullptr)
	{
		return nullptr;
	}

	return this->mCorrectedJointLocations;
}

bool OpenXRBody::isAvailable() const
{
	return mBodyTracker != nullptr;
}

XrBodyTrackerFB OpenXRBody::getBodyTrackerFB()
{
	return this->mBodyTracker;
}

bool OpenXRBody::getHeadPose(HOL::PoseLocation& pose) const
{
	const auto& head = mCorrectedJointLocations[XR_BODY_JOINT_HEAD_FB];
	if (!(head.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		|| !(head.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
	{
		return false;
	}

	pose.position = OpenXR::toEigenVector(head.pose.position);
	pose.orientation = OpenXR::toEigenQuaternion(head.pose.orientation);
	return true;
}

void OpenXRBody::preservePalmPose(HandSide side)
{
	XrBodyJointFB anchorJoint
		= canUseArmTrackingAnchor()
			  ? (side == HandSide::LeftHand ? XR_BODY_JOINT_LEFT_ARM_LOWER_FB
											: XR_BODY_JOINT_RIGHT_ARM_LOWER_FB)
			  : XR_BODY_JOINT_CHEST_FB;

	XrBodyJointLocationFB& currentAnchor = this->mJointLocations[anchorJoint];
	StoredPalmTransform& storedTransform = this->mStoredPalmTransforms[(int)side];

	XrBodyJointLocationFB& currentPalm
		= this->mJointLocations[side == HandSide::LeftHand ? XR_BODY_JOINT_LEFT_HAND_PALM_FB
														   : XR_BODY_JOINT_RIGHT_HAND_PALM_FB];

	if (!storedTransform.valid
		|| !(currentAnchor.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		|| !(currentAnchor.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
	{
		return;
	}

	applyRelativeTransform(
		currentAnchor, currentPalm, storedTransform.relativePos, storedTransform.relativeRot);

	currentPalm.locationFlags |= XR_SPACE_LOCATION_POSITION_VALID_BIT;
	currentPalm.locationFlags |= XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
	currentPalm.locationFlags &= ~XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
	currentPalm.locationFlags &= ~XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
}

void OpenXRBody::updateTrackedPalmTransform(HandSide side, XrBodyJointFB anchorJoint, XrTime time)
{
	XrBodyJointLocationFB& currentAnchor = this->mJointLocations[anchorJoint];
	XrBodyJointLocationFB& currentPalm
		= this->mJointLocations[side == HandSide::LeftHand ? XR_BODY_JOINT_LEFT_HAND_PALM_FB
														   : XR_BODY_JOINT_RIGHT_HAND_PALM_FB];
	StoredPalmTransform& storedTransform = this->mStoredPalmTransforms[(int)side];

	if (!(currentAnchor.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		|| !(currentAnchor.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
		|| !(currentPalm.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		|| !(currentPalm.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
	{
		return;
	}

	if (!mWasHandTrackingTracked[(int)side])
	{
		// Start a fresh acquisition window the first frame tracked hand data comes back.
		mHandTrackingAcquiredTime[(int)side] = time;
		mWasHandTrackingTracked[(int)side] = true;
	}

	Eigen::Vector3f currentRelativePos;
	Eigen::Quaternionf currentRelativeRot;
	calculateRelativeTransform(currentAnchor, currentPalm, currentRelativePos, currentRelativeRot);

	if (!storedTransform.valid)
	{
		storedTransform.relativePos = currentRelativePos;
		storedTransform.relativeRot = currentRelativeRot;
		storedTransform.valid = true;
		return;
	}

	float blendAlpha = 1.0f;
	float blendDurationSeconds = HOL::Config.steamvr.handTrackingResumeBlendMS / 1000.0f;
	if (blendDurationSeconds > 0.0f && mHandTrackingAcquiredTime[(int)side] > 0)
	{
		float elapsedSeconds
			= (float)(time - mHandTrackingAcquiredTime[(int)side]) / 1000000000.0f;
		blendAlpha = std::clamp(elapsedSeconds / blendDurationSeconds, 0.0f, 1.0f);
	}

	// Blend newly reacquired tracked data into the stored fallback transform so a bogus
	// one-frame hand pose cannot immediately overwrite the preserved wrist-relative pose.
	storedTransform.relativePos = storedTransform.relativePos
								  + blendAlpha * (currentRelativePos - storedTransform.relativePos);
	storedTransform.relativeRot
		= storedTransform.relativeRot.slerp(blendAlpha, currentRelativeRot);
}

bool OpenXRBody::canUseArmTrackingAnchor() const
{
	// With the oculus runtime we decide whether or not upper body tracking is enabled.
	// With VDXR, we don't know if it is enabled. However, VDXR will mark the body joints as
	// valid AND tracked when using a Quest2. With a Quest 3 where upper body is always enabled
	// they are instead untracked. Use this to determine whether or not we actually have upper body tracking.

	if (HOL::state::Runtime.isOVR)
	{
		return HOL::state::Tracking.isHighFidelityEnabled;
	}

	if (HOL::state::Runtime.isVDXR)
	{
		auto& chest = mJointLocations[XR_BODY_JOINT_CHEST_FB];
		return (chest.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
			   && !(chest.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
	}

	return false;
}

Eigen::Quaternionf OpenXRBody::getReferenceOrientation(HOL::settings::JoystickReferenceMode mode,
													   bool& valid) const
{
	auto& chest = mCorrectedJointLocations[XR_BODY_JOINT_CHEST_FB];
	auto& head = mCorrectedJointLocations[XR_BODY_JOINT_HEAD_FB];

	auto getOrientation = [](const XrBodyJointLocationFB& joint)
	{
		return Eigen::Quaternionf(joint.pose.orientation.w,
								  joint.pose.orientation.x,
								  joint.pose.orientation.y,
								  joint.pose.orientation.z);
	};

	const bool chestValid = canUseArmTrackingAnchor()
							&& (chest.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
							&& (chest.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT);
	const bool headValid = (head.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
						   && (head.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT);

	if (mode == HOL::settings::JoystickReferenceMode::Chest && chestValid)
	{
		valid = true;
		// Upper body tracking active - use chest orientation
		return getOrientation(chest);
	}

	if (mode != HOL::settings::JoystickReferenceMode::Hand && headValid)
	{
		valid = true;
		return getOrientation(head);
	}

	valid = false;
	// No valid data - return identity
	return Eigen::Quaternionf::Identity();
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
