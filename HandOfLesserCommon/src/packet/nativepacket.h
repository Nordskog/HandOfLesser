#pragma once
#include <openxr/openxr_platform.h>
#include <openxr/openxr.h>

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
		XrBool32 valid;
		XrHandEXT side;
		float trigger;
		bool triggerClick;
		bool systemClick;
	};

	// Mimics vr::DriverPose_t, but only contains the bits we care about.
	// Subject to change.
	// include packet type in packet itself so we can just memcpy the whole thing
	struct HandTransformPacket
	{
		NativePacketType packetType = NativePacketType::HandTransform;
		XrBool32 valid;
		XrHandEXT side;
		XrHandJointLocationEXT location;
		XrHandJointVelocityEXT velocity;
	};

}