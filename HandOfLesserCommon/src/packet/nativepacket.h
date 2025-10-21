#pragma once

#include "src/hand/hand.h"
#include <src/settings/settings.h>
#include "src/steamvr/skeletal_input_joints.h"
#include "src/tracking_state.h"

namespace HOL
{
	enum class NativePacketType : __int32
	{
		InvalidPacket = 100,
		HandTransform = 500,
		ControllerInput = 600,
		FloatInput = 601,
		BoolInput = 602,
		SkeletalInput = 603,
		MultimodalPose = 604,
		Settings = 700,
		State = 701
	};

	struct NativePacket
	{
		NativePacketType packetType = NativePacketType::InvalidPacket;
	};

	// these'll ultimately handle all controller inputs
	struct FloatInputPacket
	{
		NativePacketType packetType = NativePacketType::FloatInput;
		HOL::HandSide side = HOL::HandSide::LeftHand;
		char inputName[64];
		float value = 0;
	};

	struct BoolInputPacket
	{
		NativePacketType packetType = NativePacketType::BoolInput;
		HOL::HandSide side = HOL::HandSide::LeftHand;
		char inputName[64];
		bool value = 0;
	};

	struct ControllerInputPacket
	{
		NativePacketType packetType = NativePacketType::ControllerInput;
		bool valid = 0;
		HOL::HandSide side = HOL::HandSide::LeftHand;

		float triggerValue = 0.0f;
		bool triggerTouch = false;
		bool triggerClick = false;

		float gripTouch = 0.0f;
		float gripValue = 0.0f; // Like a trigger, if it had a grip trigger?
		float gripForce = 0.0f; // Squeeze? This triggers vrchat grip

		bool systemClick = false;

		float fingerCurlIndex = 0.0f;
		float fingerCurlMiddle = 0.0f;
		float fingerCurlRing = 0.0f;
		float fingerCurlPinky = 0.0f;
	};

	struct SettingsPacket
	{
		NativePacketType packetType = NativePacketType::Settings;
		HOL::settings::HandOfLesserSettings config;
	};

	struct SkeletalPacket
	{
		NativePacketType packetType = NativePacketType::SkeletalInput;
		HOL::HandSide side;
		HOL::PoseLocation
			locations[SteamVR::HandSkeletonBone::eBone_Count]; // HandSkeletonBone::eBone_Count
	};

	struct MultimodalPosePacket
	{
		NativePacketType packetType = NativePacketType::MultimodalPose;

		// Controller pose, transformed from palm position
		HOL::PoseLocation leftHandPose;
		HOL::PoseLocation rightHandPose;

		// Tracked flags
		bool leftHandTracked = false;
		bool rightHandTracked = false;
	};

	// Mimics vr::DriverPose_t, but only contains the bits we care about.
	// Subject to change.
	// include packet type in packet itself so we can just memcpy the whole thing
	struct HandTransformPacket
	{
		NativePacketType packetType = NativePacketType::HandTransform;
		bool active = false;  // Got a result
		bool valid = false;	  // Is a proper pose of some kind
		bool tracked = false; // Is directly tracked instead of inferred or frozen
		bool stale = false;
		HOL::HandSide side;
		HOL::PoseLocation location;
		HOL::PoseVelocity velocity;
	};

	struct StatePacket
	{
		NativePacketType packetType = NativePacketType::State;
		HOL::state::TrackingState state;
	};

} // namespace HOL
