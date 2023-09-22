#pragma once

#include <openvr_driver.h>

namespace HOL
{
	enum class NativePacketType : __int32
	{
		InvalidPacket = 100,
		HandTransform = 500,
	};

	struct NativePacket
	{
		NativePacketType packetType = NativePacketType::HandTransform;
	};

	// Mimics vr::DriverPose_t, but only contains the bits we care about.
	// Subject to change.
	// include packet type in packet itself so we can just memcpy the whole thing
	struct HandTransformPacket
	{
		NativePacketType packetType = NativePacketType::HandTransform;
		vr::ETrackedControllerRole hand;
		vr::HmdVector3_t position;
		vr::HmdQuaternion_t qRotation;
	};

}