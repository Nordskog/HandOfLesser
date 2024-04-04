#pragma once

#include "controller_common.h"
#include "src/core/hand_of_lesser.h"
#include <chrono>

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

		// I can't test this properly. We're already using other predictions
		float velocityMultiplier = 0.0f;
		// Controllers will vanish if velocities are invalid? not initialized?
		// Velocity numbers are bogus on the quest1 ( pure noise ), but fine on quest3?
		pose.vecVelocity[0] = packet->velocity.linearVelocity.x() * velocityMultiplier;
		pose.vecVelocity[1] = packet->velocity.linearVelocity.y() * velocityMultiplier;
		pose.vecVelocity[2] = packet->velocity.linearVelocity.z() * velocityMultiplier;
		//
		pose.vecAngularVelocity[0] = packet->velocity.angularVelocity.x() * velocityMultiplier;
		pose.vecAngularVelocity[1] = packet->velocity.angularVelocity.y() * velocityMultiplier;
		pose.vecAngularVelocity[2] = packet->velocity.angularVelocity.z() * velocityMultiplier;

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



}