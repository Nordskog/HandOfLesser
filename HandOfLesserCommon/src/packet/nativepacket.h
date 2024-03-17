#pragma once

#include "src/hand/hand.h"

namespace HOL
{
	enum class NativePacketType : __int32
	{
		InvalidPacket = 100,
		HandTransform = 500,
		ControllerInput = 600,
	};

	struct NativePacket
	{
		NativePacketType packetType = NativePacketType::InvalidPacket;
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

	// Mimics vr::DriverPose_t, but only contains the bits we care about.
	// Subject to change.
	// include packet type in packet itself so we can just memcpy the whole thing
	struct HandTransformPacket
	{
		NativePacketType packetType = NativePacketType::HandTransform;
		bool active = false;
		bool valid = false;
		HOL::HandSide side;
		HOL::PoseLocation location;
		HOL::PoseVelocity velocity;
	};

} // namespace HOL
