#include "openxr_hand.h"
#include "XrUtils.h"
#include <HandOfLesserCommon.h>
#include "HandTrackingInterface.h"
#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include <iostream>
#include <utility>

#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace HOL::OpenXR;

OpenXRHand::OpenXRHand(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	init(session, side);
}

void OpenXRHand::init(xr::UniqueDynamicSession& session, XrHandEXT side)
{
	this->mSide = side;
	HandTrackingInterface::createHandTracker(session, side, this->mHandTracker);
}

void OpenXRHand::calculateCurlSplay()
{
	const auto getJointOrientation = [&](XrHandJointEXT joint)
	{ return toEigenQuaternion(this->mJointLocations[joint].pose.orientation); };

	const auto getRootOpenXRJointForFinger = [&](FingerType joint)
	{
		switch (joint)
		{
			case FingerType::FingerThumb:
				return XrHandJointEXT::XR_HAND_JOINT_WRIST_EXT; // Thumb joint calculated from wrist
			case FingerType::FingerIndex:
				return XrHandJointEXT::XR_HAND_JOINT_INDEX_METACARPAL_EXT;
			case FingerType::FingerMiddle:
				return XrHandJointEXT::XR_HAND_JOINT_MIDDLE_METACARPAL_EXT;
			case FingerType::FingerRing:
				return XrHandJointEXT::XR_HAND_JOINT_RING_METACARPAL_EXT;
			case FingerType::FingerLittle:
				return XrHandJointEXT::XR_HAND_JOINT_LITTLE_METACARPAL_EXT;
		}
	};

	// Get the orientation of the 4 relevant joints for each finger
	// the thumb has a special case
	const auto getRawOrientation = [&](FingerType finger, Eigen::Quaternionf rawOrientationOut[])
	{
		XrHandJointEXT rootJoint = getRootOpenXRJointForFinger(finger); // METACARPAL

		if (finger == FingerType::FingerThumb)
		{
			// TOOD
		}
		else
		{
			// openxr joints are hierarchical, so we can +1 until we reach the tip of the finger
			for (int i = 0; i < 4; i++)
			{
				rawOrientationOut[i] = getJointOrientation((XrHandJointEXT)(rootJoint + i));
			}
		}
	};

	// Reuse array to store raw orientations of joints
	Eigen::Quaternionf rawOrientation[4];

	// For each finger
	for (int i = 0; i < FingerType::FingerType_MAX; i++)
	{
		FingerBend* bend = &this->handPose.fingers[i];
		FingerType finger = (FingerType)i;
		getRawOrientation(finger, rawOrientation);

		// For each joint + next joint
		for (int j = 0; j < 3; j++)
		{
			// Pass current joint, and the next joint, starting from METACARPAL.
			bend->bend[j] = computeCurl(rawOrientation[j], rawOrientation[j + 1]);

			// Curling inwards is negative, outwards positive, with 0 being a straight finger.
			// this is a bit unintuitive, so let's flip it.
			bend->bend[j] *= -1.0f;
		}

		// Between metacarpal and proximal. No flipping this.
		bend->bend[FingerBendType::Splay] = computeSplay(rawOrientation[0], rawOrientation[1]);
	}
}

void OpenXRHand::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	HandTrackingInterface::locateHandJoints(this->mHandTracker,
											space,
											time,
											this->mJointLocations,
											this->mJointVelocities,
											&this->mAimState);

	auto palmLocation = this->mJointLocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];
	auto palmVelocity = this->mJointVelocities[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	this->handPose.palmLocation.position = toEigenVector(palmLocation.pose.position);
	this->handPose.palmLocation.orientation = toEigenQuaternion(palmLocation.pose.orientation);

	// Orientation is not going to be set without position for hand tracking.
	this->handPose.poseValid = (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
							   == XR_SPACE_LOCATION_POSITION_VALID_BIT;

	this->handPose.palmVelocity.linearVelocity = toEigenVector(palmVelocity.linearVelocity);
	this->handPose.palmVelocity.angularVelocity = toEigenVector(palmVelocity.angularVelocity);

	// Transform palm ( not actually same as grip ) to index controller tracker position
	Eigen::Vector3f gripRotationOffset = Eigen::Vector3f(63, 0, -99);
	Eigen::Vector3f gripTranslationOffset = Eigen::Vector3f(0.021, 0, -0.114);

	// Add configurable offset
	Eigen::Vector3f mainRotationOffset = gripRotationOffset + HOL::settings::OrientationOffset;
	Eigen::Vector3f mainTranslationOffset = gripTranslationOffset + HOL::settings::PositionOffset;

	if (this->mSide == XR_HAND_LEFT_EXT)
	{
		// Base offsets are for the left hand
	}
	else
	{
		mainRotationOffset = flipHandRotation(mainRotationOffset);
		mainTranslationOffset = flipHandTranslation(mainTranslationOffset);
	}

	////////////
	// Rot
	///////////

	Eigen::Quaternionf rawOrientation = toEigenQuaternion(palmLocation.pose.orientation);
	Eigen::Quaternionf rotationOffsetQuat
		= HOL::quaternionFromEulerAnglesDegrees(mainRotationOffset);

	this->handPose.palmLocation.orientation = rawOrientation * rotationOffsetQuat;

	//////////////
	// Trans
	/////////////

	Eigen::Vector3f rawPosition = toEigenVector(palmLocation.pose.position);
	Eigen::Vector3f rotationOffsetLocal
		= this->handPose.palmLocation.orientation * mainTranslationOffset;
	this->handPose.palmLocation.position = rawPosition + rotationOffsetLocal;

	{
		HOL::display::HandTransform[this->mSide].rawPose.position = rawPosition;
		HOL::display::HandTransform[this->mSide].rawPose.orientation = rawOrientation;

		HOL::display::HandTransform[this->mSide].finalPose.position
			= this->handPose.palmLocation.position;
		HOL::display::HandTransform[this->mSide].finalPose.orientation
			= this->handPose.palmLocation.orientation;

		HOL::display::HandTransform[this->mSide].finalTranslationOffset = mainTranslationOffset;
		HOL::display::HandTransform[this->mSide].finalOrientationOffset = mainRotationOffset;
	}

	this->calculateCurlSplay();
}
