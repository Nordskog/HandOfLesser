#include "openxr_hand.h"
#include "XrUtils.h"
#include "xr_hand_utils.h"
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

XrHandJointLocationEXT* OpenXRHand::getLastJointLocations()
{
	return this->mJointLocations;
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

	// Get the orientation of the 4 relevant joints for each finger
	// the thumb has a special case
	const auto getRawOrientation = [&](FingerType finger, Eigen::Quaternionf rawOrientationOut[])
	{
		XrHandJointEXT rootJoint = OpenXR::getRootJoint(finger); // METACARPAL or wrist ( thumb )

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
				rawOffset.x(), rawOffset.y(), rawOffset.z());

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
		//bend->bend[FingerBendType::Splay] = computeSplay(rawOrientation[0], rawOrientation[1]);
		bend->bend[FingerBendType::Splay]
			= computeUncurledSplay(rawOrientation[0], rawOrientation[1], -bend->bend[0]);	// Note that we flip it again

		if (Config.vrchat.useUnityHumanoidSplay)
		{
			// Special values for unity's broken humanoid rig
			// Only splay is different
			Eigen::Quaternionf palmRot
				= getJointOrientation(XrHandJointEXT::XR_HAND_JOINT_PALM_EXT);

			if (finger == FingerType::FingerThumb)
			{
				palmRot = rawOrientation[0];
			}

			Eigen::Vector3f knucklePos = getJointPosition(OpenXR::getFirstFingerJoint(finger));
			Eigen::Vector3f splayRefPos
				= getJointPosition((XrHandJointEXT)(OpenXR::getFirstFingerJoint(finger) + 1));

			bend->setSplay(computeHumanoidSplay(palmRot, knucklePos, splayRefPos));
		}
		else
		{
			// URGHHHHHHH




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
																	&this->aimState);

	auto palmLocation = this->mJointLocations[XrHandJointEXT::XR_HAND_JOINT_PALM_EXT];

	// Orientation is not going to be set without position for hand tracking.
	this->handPose.poseValid = (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
							   == XR_SPACE_LOCATION_POSITION_VALID_BIT;
	this->handPose.poseTracked
		= (palmLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
		  == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

	// Never stale if invalid?
	this->handPose.poseStale = false;

	if (HOL::Config.general.forceInactive)
	{
		this->handPose.poseValid = false;
		this->handPose.active = false;
		this->handPose.poseTracked = false;
	}

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

			// We have a base offset for each controller type, which for the time being
			// has been configured to match what VD handtracking gives you.
			// The user-configurable offset is applied in addition to this.
			auto controllerOffset = HOL::getControllerBaseOffset(Config.handPose.controllerType);

			// Matches to controller position, matching what VD does
			Eigen::Vector3f controllerRotationOffset = controllerOffset.orientation;
			Eigen::Vector3f controllerTranslationOffset = controllerOffset.position;

			// Additional user-configurable offset, so it can be applied
			// either directly to the pose we submit, or used as an offset in the driver.
			Eigen::Vector3f userRotationOffset = Config.handPose.orientationOffset;
			Eigen::Vector3f userTranslationOffset = Config.handPose.positionOffset;

			if (this->mSide == HandSide::LeftHand)
			{
				// Base offsets are for the left hand
			}
			else
			{
				controllerRotationOffset = flipHandRotation(controllerRotationOffset);
				controllerTranslationOffset = flipHandTranslation(controllerTranslationOffset);

				userRotationOffset = flipHandRotation(userRotationOffset);
				userTranslationOffset = flipHandTranslation(userTranslationOffset);
			}

			//////////////////////
			// Controller offset
			//////////////////////

			// Remember to apply translation then rotation, always

			this->handPose.palmLocation.position
				= HOL::translateLocal(this->handPose.palmLocation.position,
									  this->handPose.palmLocation.orientation,
									  controllerTranslationOffset);

			this->handPose.palmLocation.orientation
				= HOL::rotateLocal(this->handPose.palmLocation.orientation,
								   HOL::quaternionFromEulerAnglesDegrees(controllerRotationOffset));

			////////////////
			// User offset
			///////////////

			this->handPose.palmLocation.position
				= HOL::translateLocal(this->handPose.palmLocation.position,
									  this->handPose.palmLocation.orientation,
									  userTranslationOffset);

			// We need to perform this completely separate for the offset to match when
			// applied to the existing hand-tracking pose supplied by VD when in Offset mode.
			this->handPose.palmLocation.orientation
				= HOL::rotateLocal(this->handPose.palmLocation.orientation,
								   HOL::quaternionFromEulerAnglesDegrees(userRotationOffset));

			//////////////////////
			// Finger movement
			//////////////////////

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

				HOL::display::HandTransform[this->mSide].rawPose.position = newPalmPosition;
				HOL::display::HandTransform[this->mSide].rawPose.orientation = newPalmOrientation;

				HOL::display::HandTransform[this->mSide].finalPose.position
					= this->handPose.palmLocation.position;
				HOL::display::HandTransform[this->mSide].finalPose.orientation
					= this->handPose.palmLocation.orientation;

				HOL::display::HandTransform[this->mSide].finalTranslationOffset
					= controllerTranslationOffset;
				HOL::display::HandTransform[this->mSide].finalOrientationOffset
					= controllerRotationOffset;

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
