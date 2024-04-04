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

using namespace HOL;
using namespace HOL::OpenXR;


void OpenXRHand::init(xr::UniqueDynamicSession& session, HOL::HandSide side)
{
	this->mSide = side;
	HandTrackingInterface::createHandTracker(session, toOpenXRHandSide(side), this->mHandTracker);
}

void OpenXRHand::calculateCurlSplay()
{
	if (!this->handPose.poseTracked)
	{
		// Leave previous pose if hand not tracked
		return;
	}

	const auto getJointOrientation = [&](XrHandJointEXT joint)
	{ return toEigenQuaternion(this->mJointLocations[joint].pose.orientation); };

	const auto getJointPosition = [&](XrHandJointEXT joint)
	{ return toEigenVector(this->mJointLocations[joint].pose.position); };

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

	const auto getFirstJoint = [&](FingerType joint)
	{
		switch (joint)
		{
			case FingerType::FingerThumb:
				return XrHandJointEXT::XR_HAND_JOINT_THUMB_METACARPAL_EXT;
			case FingerType::FingerIndex:
				return XrHandJointEXT::XR_HAND_JOINT_INDEX_PROXIMAL_EXT;
			case FingerType::FingerMiddle:
				return XrHandJointEXT::XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT;
			case FingerType::FingerRing:
				return XrHandJointEXT::XR_HAND_JOINT_RING_PROXIMAL_EXT;
			case FingerType::FingerLittle:
				return XrHandJointEXT::XR_HAND_JOINT_LITTLE_PROXIMAL_EXT;
		}
	};

	// Get the orientation of the 4 relevant joints for each finger
	// the thumb has a special case
	const auto getRawOrientation = [&](FingerType finger, Eigen::Quaternionf rawOrientationOut[])
	{
		XrHandJointEXT rootJoint = getRootOpenXRJointForFinger(finger); // METACARPAL

		// openxr joints are hierarchical, so we can +1 until we reach the tip of the finger
		for (int i = 0; i < 4; i++)
		{
			rawOrientationOut[i] = getJointOrientation((XrHandJointEXT)(rootJoint + i));
		}

		// The root bone for the thumb would be the bone before the metacarpal,
		// which doesn't exist; the next bone is the wrist.
		// Use wrist orientation, but offset by user input to calibrate correctly.
		if (finger == FingerType::FingerThumb)
		{
			Eigen::Quaternionf wristOrientation = getJointOrientation(rootJoint);
			Eigen::Vector3f rawOffset = Config.fingerBend.ThumbAxisOffset;
			if (this->mSide == HandSide::RightHand)
			{
				// Input values are for left hand
				rawOffset = flipHandRotation(rawOffset);
			}


			// Z axis is probably all you want to adjust
			Eigen::Quaternionf baseOffset = HOL::quaternionFromEulerAnglesDegrees(
				rawOffset.x(), 
				rawOffset.y(), 
				rawOffset.z());

			rawOrientationOut[0] = wristOrientation * (baseOffset);
		}
		else
		{
			// For the sake of the humanoid rig, using the palm as a reference is better
			rawOrientationOut[0] = getJointOrientation(XrHandJointEXT::XR_HAND_JOINT_PALM_EXT);
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

		if (Config.vrchat.useUnityHumanoidSplay)
		{
			// Special values for unity's broken humanoid rig
			// Only splay is different
			Eigen::Quaternionf palmRot = getJointOrientation(XrHandJointEXT::XR_HAND_JOINT_PALM_EXT);

			if (finger == FingerType::FingerThumb)
			{
				palmRot = rawOrientation[0];
			}

			Eigen::Vector3f knucklePos = getJointPosition(getFirstJoint(finger));
			Eigen::Vector3f tipPos = getJointPosition((XrHandJointEXT)(getFirstJoint(finger)+1));

			bend->setSplay(computeHumanoidSplay(palmRot, knucklePos, tipPos));
		}
	}
}

void OpenXRHand::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->handPose.active = HandTrackingInterface::locateHandJoints(this->mHandTracker,
											space,
											time,
											this->mJointLocations,
											this->mJointVelocities,
											&this->mAimState);

	auto palmLocation = this->mJointLocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	// Orientation is not going to be set without position for hand tracking.
	this->handPose.poseValid = (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
							   == XR_SPACE_LOCATION_POSITION_VALID_BIT;
	this->handPose.poseTracked
		= (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		  == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

	// Never stale if invalid?
	this->handPose.poseStale = false;


	if (this->handPose.poseValid)
	{
		Eigen::Vector3f newPalmPosition = toEigenVector(palmLocation.pose.position);
		Eigen::Quaternionf newPalmOrientation = toEigenQuaternion(palmLocation.pose.orientation);

		//////////////
		// Staleness
		//////////////

		// Don't bother doing anything with stale data.
		// VDXR updates at intervals of 7-16ms, Airlink updates constantly.
		// Prediction also does nothing for VDXR.
		this->handPose.poseStale = this->mPrevRawPose.position.isApprox(newPalmPosition);

		if (!this->handPose.poseStale)
		{
			//////////////////////////////////
			// Update transform and velocity
			//////////////////////////////////
			this->handPose.palmLocation.position = newPalmPosition;
			this->handPose.palmLocation.orientation = newPalmOrientation;

			auto palmVelocity = this->mJointVelocities[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

			this->handPose.palmVelocity.linearVelocity = toEigenVector(palmVelocity.linearVelocity);
			this->handPose.palmVelocity.angularVelocity
				= toEigenVector(palmVelocity.angularVelocity);

			this->handPose.palmVelocity.linearVelocity
				*= HOL::Config.general.linearVelocityMultiplier;
			this->handPose.palmVelocity.angularVelocity
				*= HOL::Config.general.angularVelocityMultiplier;

			/////////////
			// Offsets
			/////////////

			// Transform palm ( not actually same as grip ) to index controller tracker position
			Eigen::Vector3f gripRotationOffset = Eigen::Vector3f(63, 0, -99);
			Eigen::Vector3f gripTranslationOffset = Eigen::Vector3f(0.021, 0, -0.114);

			// Add configurable offset
			Eigen::Vector3f mainRotationOffset
				= gripRotationOffset + Config.handPose.OrientationOffset;
			Eigen::Vector3f mainTranslationOffset
				= gripTranslationOffset + Config.handPose.PositionOffset;

			if (this->mSide == HandSide::LeftHand)
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

			Eigen::Quaternionf rawOrientation = this->handPose.palmLocation.orientation;
			Eigen::Quaternionf rotationOffsetQuat
				= HOL::quaternionFromEulerAnglesDegrees(mainRotationOffset);

			this->handPose.palmLocation.orientation = rawOrientation * rotationOffsetQuat;

			//////////////
			// Trans
			/////////////

			Eigen::Vector3f rawPosition = this->handPose.palmLocation.position;
			Eigen::Vector3f positionOffsetLocal
				= this->handPose.palmLocation.orientation * mainTranslationOffset;
			this->handPose.palmLocation.position = rawPosition + positionOffsetLocal;

			this->calculateCurlSplay();

			/////////////
			// Prev
			/////////////

			// Set prev
			this->mPrevRawPose.position = newPalmPosition;
			this->mPrevRawPose.orientation = newPalmOrientation;

			///////////////////////////
			// Update display values
			///////////////////////////
			{
				// Hand orientation

				HOL::display::HandTransform[this->mSide].rawPose.position = rawPosition;
				HOL::display::HandTransform[this->mSide].rawPose.orientation = rawOrientation;

				HOL::display::HandTransform[this->mSide].finalPose.position
					= this->handPose.palmLocation.position;
				HOL::display::HandTransform[this->mSide].finalPose.orientation
					= this->handPose.palmLocation.orientation;

				HOL::display::HandTransform[this->mSide].finalTranslationOffset
					= mainTranslationOffset;
				HOL::display::HandTransform[this->mSide].finalOrientationOffset
					= mainRotationOffset;

				// Finger curl
				for (int i = 0; i < FingerType_MAX; i++)
				{
					// Turns out you cannot assign an array to an array
					HOL::display::FingerTracking[this->mSide].rawBend[i]
						= this->handPose.fingers[i];
				}
			}
		}
	}

	///////////////////////////
	// Update display values
	///////////////////////////

	{
		HOL::display::HandTransform[this->mSide].active = this->handPose.active;
		HOL::display::HandTransform[this->mSide].positionValid = this->handPose.poseValid;
		HOL::display::HandTransform[this->mSide].positionTracked = this->handPose.poseTracked;
	}
}
