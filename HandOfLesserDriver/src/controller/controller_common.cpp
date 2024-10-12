#pragma once

#include "controller_common.h"
#include "src/core/hand_of_lesser.h"
#include <chrono>
#include <driverlog.h>

namespace HOL::ControllerCommon
{
	static std::default_random_engine JitterGenerator = std::default_random_engine(0);
	static std::uniform_real_distribution<float> JitterDistribution
		= std::uniform_real_distribution<float>(0, 0.0001);

	

	vr::DriverPose_t generatePose(HOL::HandTransformPacket* packet, bool deviceConnected)
	{
		// Let's retrieve the Hmd pose to base our controller pose off.

		// First, initialize the struct that we'll be submitting to the runtime to tell it we've
		// updated our pose.
		vr::DriverPose_t pose = {0};

		// We can request a prediction from openXR as well
		// At the moment using steam for anything does not end well,
		// especially because velocities are mostly bogus.
		pose.poseTimeOffset = HandOfLesser::Config.general.steamPoseTimeOffset;

		// These need to be set to be valid quaternions. The device won't appear otherwise.
		pose.qWorldFromDriverRotation.w = 1.f;
		pose.qDriverFromHeadRotation.w = 1.f;
		// I guess this would be to align coordinate systems if they were offset.
		// Probably won't need that for quest

		// copy our position to our pose
		pose.vecPosition[0] = packet->location.position.x();
		pose.vecPosition[1] = packet->location.position.y();
		pose.vecPosition[2] = packet->location.position.z();

		pose.qRotation.w = packet->location.orientation.w();
		pose.qRotation.x = packet->location.orientation.x();
		pose.qRotation.y = packet->location.orientation.y();
		pose.qRotation.z = packet->location.orientation.z();

		// Ideally we would supply velocities with our poses so SteamVR can
		// do extra prediction and make up for low samples (I .e.g VDXR ).
		// Unfortunately the velocity values are too noisy, and if we supply them
		// everything goes to shit. Use OpenXR predicition instead where it works.

		// Controllers will vanish if velocities are invalid? not initialized?
		pose.vecVelocity[0] = packet->velocity.linearVelocity.x();
		pose.vecVelocity[1] = packet->velocity.linearVelocity.y();
		pose.vecVelocity[2] = packet->velocity.linearVelocity.z();
		//
		pose.vecAngularVelocity[0] = packet->velocity.angularVelocity.x();
		pose.vecAngularVelocity[1] = packet->velocity.angularVelocity.y();
		pose.vecAngularVelocity[2] = packet->velocity.angularVelocity.z();

		// Acceleration being wrong can make controllers not appear
		pose.vecAcceleration[0] = 0;
		pose.vecAcceleration[1] = 0;
		pose.vecAcceleration[2] = 0;

		pose.vecAngularAcceleration[0] = 0;
		pose.vecAngularAcceleration[1] = 0;
		pose.vecAngularAcceleration[2] = 0;

		// The pose we provided is valid.
		// This should be set is
		pose.poseIsValid = true;

		// Our device is always connected.
		// In reality with physical devices, when they get disconnected,
		// set this to false and icons in SteamVR will be updated to show the device is disconnected
		pose.deviceIsConnected = true;

		// The state of our tracking. For our virtual device, it's always going to be ok,
		// but this can get set differently to inform the runtime about the state of the device's
		// tracking and update the icons to inform the user accordingly.
		pose.result // What state to use for disconnected?
			= deviceConnected ? vr::TrackingResult_Running_OK : vr::TrackingResult_Uninitialized;

		return pose;
	}

	vr::DriverPose_t addJitter(const vr::DriverPose_t& existingPose)
	{
		// VRChat is stupid and ignores all status values passed to it by SteamVR
		// in favor is checking whether position values change. If they remain completely
		// static for a short period of time it decides tracking has been lost and moves
		// your arms to the sides. Idiots.
		vr::DriverPose_t jitteredPose = existingPose;

		jitteredPose.vecPosition[0] += JitterDistribution(JitterGenerator);
		jitteredPose.vecPosition[1] += JitterDistribution(JitterGenerator);
		jitteredPose.vecPosition[2] += JitterDistribution(JitterGenerator);

		return jitteredPose;
	}

	void offsetPose(vr::DriverPose_t& existingPose,
					HOL::HandSide side,
					Eigen::Vector3f translationOffset,
					Eigen::Vector3f rotationOffset)
	{

		if (side == HandSide::LeftHand)
		{
			// Base offsets are for the left hand
		}
		else
		{
			rotationOffset = flipHandRotation(rotationOffset);
			translationOffset = flipHandTranslation(translationOffset);
		}

		/////////////////////
		// Existing values
		////////////////////

		Eigen::Quaternionf poseRotation = Eigen::Quaternionf(existingPose.qRotation.w,
															 existingPose.qRotation.x,
															 existingPose.qRotation.y,
															 existingPose.qRotation.z);

		Eigen::Vector3f poseTranslation = Eigen::Vector3f(
			existingPose.vecPosition[0], existingPose.vecPosition[1], existingPose.vecPosition[2]);

		///////////////////////////
		// Apply driver offsets
		///////////////////////////

		// Apply the DriverFromHead offset, rather than letting OVR do it.
		// This allows us to apply the same offset to both possess and offset modes.
		// SteamVR wants the original sensor position for applying velocities and stuff,
		// but as of writing there is no hand-tracking solution that provides useful
		// velocities anyway, so we just supply the final pose.
		// In theory you could apply the driver offset, apply our offsets, then
		// apply the inverse of the driver offset. Do that if we ever need velocities.

		// offsets are expected to be applied translation, then rotation.
		// we should try to adhere to this with all our offsets too.

		Eigen::Vector3f vecDriverFromHead
			= Eigen::Vector3f(existingPose.vecDriverFromHeadTranslation[0],
							  existingPose.vecDriverFromHeadTranslation[1],
							  existingPose.vecDriverFromHeadTranslation[2]);
		Eigen::Quaternionf qDriverFromHead
			= Eigen::Quaternionf(existingPose.qDriverFromHeadRotation.w,
								 existingPose.qDriverFromHeadRotation.x,
								 existingPose.qDriverFromHeadRotation.y,
								 existingPose.qDriverFromHeadRotation.z);

		poseTranslation = HOL::translateLocal(poseTranslation, poseRotation, vecDriverFromHead);
		poseRotation = poseRotation * qDriverFromHead;

		// Clear in pose
		existingPose.vecDriverFromHeadTranslation[0] = 0;
		existingPose.vecDriverFromHeadTranslation[1] = 0;
		existingPose.vecDriverFromHeadTranslation[2] = 0;

		existingPose.qDriverFromHeadRotation.w = 1;
		existingPose.qDriverFromHeadRotation.x = 0;
		existingPose.qDriverFromHeadRotation.y = 0;
		existingPose.qDriverFromHeadRotation.z = 0;

		////////////////////
		// Apply our offets
		////////////////////

		poseTranslation = HOL::translateLocal(poseTranslation, poseRotation, translationOffset);
		poseRotation
			= HOL::rotateLocal(poseRotation, HOL::quaternionFromEulerAnglesDegrees(rotationOffset));

		//////////
		// Assign
		//////////

		existingPose.vecPosition[0] = poseTranslation.x();
		existingPose.vecPosition[1] = poseTranslation.y();
		existingPose.vecPosition[2] = poseTranslation.z();

		existingPose.qRotation.w = poseRotation.w();
		existingPose.qRotation.x = poseRotation.x();
		existingPose.qRotation.y = poseRotation.y();
		existingPose.qRotation.z = poseRotation.z();
	}

	vr::VRBoneTransform_t poseLocationToBoneTransform(HOL::PoseLocation& location)
	{
		vr::VRBoneTransform_t trans;
		trans.position.v[0] = location.position.x();
		trans.position.v[1] = location.position.y();
		trans.position.v[2] = location.position.z();
		trans.position.v[3] = 1.0f; // I guess?

		trans.orientation.w = location.orientation.w();
		trans.orientation.x = location.orientation.x();
		trans.orientation.y = location.orientation.y();
		trans.orientation.z = location.orientation.z();	

		return trans;
	}

} // namespace HOL::ControllerCommon