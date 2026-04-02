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
		pose.poseTimeOffset = HandOfLesser::Config.steamvr.steamPoseTimeOffset;

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

	vr::DriverPose_t generateDisconnectedPose()
	{
		// Tells SteamVR that the controller is disconnected
		vr::DriverPose_t pose = {0};

		pose.deviceIsConnected = false;
		pose.poseIsValid = false;

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

		// Native controller poses often keep part of their transform in DriverFromHead.
		// SteamVR can use that original structure for its own prediction, so in fallback-only
		// mode we must not bake it into vecPosition/qRotation and clear it out.
		// Instead, convert to the final native pose, apply our calibration offset there,
		// then solve back to the local pose while preserving DriverFromHead unchanged.

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

		////////////////////
		// Apply our offsets
		////////////////////

		// Offsets are expected to be applied translation, then rotation.
		// We should try to adhere to this with all our offsets too.
		poseTranslation = HOL::translateLocal(poseTranslation, poseRotation, translationOffset);
		poseRotation
			= HOL::rotateLocal(poseRotation, HOL::quaternionFromEulerAnglesDegrees(rotationOffset));

		//////////////////////////////////////
		// Restore the original pose structure
		//////////////////////////////////////

		// Leave the native DriverFromHead data and native velocities untouched so SteamVR can
		// keep predicting this as the original tracked controller instead of as a rebased pose.
		Eigen::Quaternionf localPoseRotation = poseRotation * qDriverFromHead.inverse();
		Eigen::Vector3f localPoseTranslation
			= poseTranslation - (localPoseRotation * vecDriverFromHead);

		//////////
		// Assign
		//////////

		existingPose.vecPosition[0] = localPoseTranslation.x();
		existingPose.vecPosition[1] = localPoseTranslation.y();
		existingPose.vecPosition[2] = localPoseTranslation.z();

		existingPose.qRotation.w = localPoseRotation.w();
		existingPose.qRotation.x = localPoseRotation.x();
		existingPose.qRotation.y = localPoseRotation.y();
		existingPose.qRotation.z = localPoseRotation.z();
	}

	void applyDriverOffset(Eigen::Vector3f& posePosition,
						   Eigen::Quaternionf& poseRotation,
						   const vr::DriverPose_t& pose)
	{
		Eigen::Vector3f vecDriverFromHead = Eigen::Vector3f(pose.vecDriverFromHeadTranslation[0],
															pose.vecDriverFromHeadTranslation[1],
															pose.vecDriverFromHeadTranslation[2]);
		Eigen::Quaternionf qDriverFromHead = Eigen::Quaternionf(pose.qDriverFromHeadRotation.w,
																pose.qDriverFromHeadRotation.x,
																pose.qDriverFromHeadRotation.y,
																pose.qDriverFromHeadRotation.z);

		posePosition = HOL::translateLocal(posePosition, poseRotation, vecDriverFromHead);
		poseRotation = poseRotation * qDriverFromHead;
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

	void buildSkeletalPoseFromPacket(
		const HOL::SkeletalPacket& packet,
		vr::VRBoneTransform_t outPose[SteamVR::HandSkeletonBone::eBone_Count])
	{
		HOL::SkeletalPacket workingPacket = packet;

		if (workingPacket.side == HandSide::RightHand)
		{
			HOL::PoseLocation& wristJoint
				= workingPacket.locations[SteamVR::HandSkeletonBone::eBone_Wrist];

			wristJoint.position.x() *= -1.f;
			wristJoint.position.y() *= -1.f;

			wristJoint.orientation.x() *= -1.f;
			wristJoint.orientation.y() *= -1.f;
		}

		for (int i = 1; i < SteamVR::HandSkeletonBone::eBone_Count; i++)
		{
			auto bone = static_cast<SteamVR::HandSkeletonBone>(i);
			auto& joint = workingPacket.locations[bone];

			std::swap(joint.position.x(), joint.position.z());
			joint.position.z() *= -1.f;

			std::swap(joint.orientation.x(), joint.orientation.z());
			joint.orientation.z() *= -1.f;

			if (workingPacket.side == HandSide::LeftHand)
			{
				joint.position.x() *= -1.f;
				joint.position.y() *= -1.f;

				joint.orientation.x() *= -1.f;
				joint.orientation.y() *= -1.f;
			}
		}

		const SteamVR::HandSkeletonBone startingJoint[5]
			= {SteamVR::HandSkeletonBone::eBone_Thumb0,
			   SteamVR::HandSkeletonBone::eBone_IndexFinger0,
			   SteamVR::HandSkeletonBone::eBone_MiddleFinger0,
			   SteamVR::HandSkeletonBone::eBone_RingFinger0,
			   SteamVR::HandSkeletonBone::eBone_PinkyFinger0};

		const int fingerJointCount[5] = {4, 5, 5, 5, 5};

		const Eigen::Quaternionf leftMagic(0.5f, 0.5f, -0.5f, 0.5f);
		const Eigen::Quaternionf rightMagic(-0.5f, 0.5f, -0.5f, -0.5f);
		for (int finger = 0; finger < 5; finger++)
		{
			auto& joint = workingPacket.locations[startingJoint[finger]];
			const Eigen::Quaternionf magic
				= workingPacket.side == HandSide::LeftHand ? leftMagic : rightMagic;

			joint.orientation = magic * joint.orientation;
			joint.position = magic * joint.position;
		}

		auto& wristJoint = workingPacket.locations[SteamVR::HandSkeletonBone::eBone_Wrist];
		for (int finger = 0; finger < 5; finger++)
		{
			SteamVR::HandSkeletonBone firstJoint = startingJoint[finger];
			int childCount = fingerJointCount[finger] - 1;

			auto auxIndex = static_cast<SteamVR::HandSkeletonBone>(
				SteamVR::HandSkeletonBone::eBone_Aux_Thumb + finger);
			auto& auxJoint = workingPacket.locations[auxIndex];
			auxJoint.position = wristJoint.position;
			auxJoint.orientation = wristJoint.orientation;

			for (int j = 0; j < childCount; j++)
			{
				int childIndex = static_cast<int>(firstJoint) + 1 + j;
				auto childBone = static_cast<SteamVR::HandSkeletonBone>(childIndex);
				auto& child = workingPacket.locations[childBone];
				auxJoint.position += auxJoint.orientation * child.position;
				auxJoint.orientation = auxJoint.orientation * child.orientation;
			}
		}

		for (int i = 0; i < SteamVR::HandSkeletonBone::eBone_Count; i++)
		{
			outPose[i] = poseLocationToBoneTransform(workingPacket.locations[i]);
		}
	}
} // namespace HOL::ControllerCommon
