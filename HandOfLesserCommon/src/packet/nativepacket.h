#pragma once

#include "src/hand/hand.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <src/settings/settings.h>
#include "src/steamvr/skeletal_input_joints.h"
#include "src/state/state.h"
#include "src/controller/controller.h"

namespace HOL
{
	enum class NativePacketType : __int32
	{
		HandTransform = 0,
		ControllerInput,
		FloatInput,
		BoolInput,
		SkeletalInput,
		MultimodalPose,
		BodyTrackerPose,
		Settings,
		State,
		DriverInitialized,
		DriverStatus,
		DeviceState,
		AppInitialized,
		PacketType_MAX,
		InvalidPacket = PacketType_MAX
	};

	struct NativePacket
	{
		NativePacketType packetType = NativePacketType::InvalidPacket;
	};

	inline constexpr size_t SettingsPacketSize = 8196;
	inline constexpr size_t SettingsPacketJsonBufferSize = SettingsPacketSize - sizeof(NativePacket);

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
		char jsonData[SettingsPacketJsonBufferSize] = {}; // Settings serialized as JSON string
	};
	static_assert(sizeof(SettingsPacket) == SettingsPacketSize);

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
		HOL::HandSide side = HandSide::HandSide_MAX;
		HOL::PoseLocation location;
		HOL::PoseVelocity velocity;
	};

	struct BodyTrackerPosePacket
	{
		NativePacketType packetType = NativePacketType::BodyTrackerPose;
		HOL::BodyTrackerRole role;
		HOL::PoseLocation location;
		HOL::PoseVelocity velocity;
		bool active = false;
		bool valid = false;
		bool tracked = false;
	};

	struct StatePacket
	{
		NativePacketType packetType = NativePacketType::State;
		HOL::state::TrackingState tracking;
		HOL::state::RuntimeState runtime;
	};

	struct DriverInitializedPacket
	{
		NativePacketType packetType = NativePacketType::DriverInitialized;
		char driverVersion[64] = "0.1.0"; // For future version checks
		uint32_t capabilities = 0;		  // Bitfield for future features
	};

	struct DriverStatusPacket
	{
		NativePacketType packetType = NativePacketType::DriverStatus;
		bool emulatedControllersActive = false;
		bool hasNormalControllers = false;
		bool hasHandTrackingControllers = false;
		int hookedControllerCount = 0;
		int emulatedTrackerCount = 0;
	};

	struct DeviceStatePacket
	{
		NativePacketType packetType = NativePacketType::DeviceState;
		char serial[128] = {};
		vr::ETrackedDeviceClass role = vr::TrackedDeviceClass_Invalid;
		vr::EVRSkeletalTrackingLevel trackingLevel = vr::VRSkeletalTracking_Estimated;
		bool nativePoseIsValid = false;
		bool nativeDeviceIsConnected = false;
		vr::ETrackingResult nativeTrackingResult = vr::TrackingResult_Uninitialized;
		uint64_t nativePoseAgeMs = 0;
	};

	struct AppInitializedPacket
	{
		NativePacketType packetType = NativePacketType::AppInitialized;
		char appVersion[64] = "0.1.0"; // For future version checks
		uint32_t capabilities = 0;	   // Bitfield for future features
	};

	inline constexpr std::array<size_t, static_cast<size_t>(NativePacketType::PacketType_MAX)>
		NativePacketSizes{{
			sizeof(HandTransformPacket),
			sizeof(ControllerInputPacket),
			sizeof(FloatInputPacket),
			sizeof(BoolInputPacket),
			sizeof(SkeletalPacket),
			sizeof(MultimodalPosePacket),
			sizeof(BodyTrackerPosePacket),
			sizeof(SettingsPacket),
			sizeof(StatePacket),
			sizeof(DriverInitializedPacket),
			sizeof(DriverStatusPacket),
			sizeof(DeviceStatePacket),
			sizeof(AppInitializedPacket),
		}};

	inline bool isValidNativePacket(NativePacketType type, size_t length)
	{
		size_t index = static_cast<size_t>(type);
		if (index >= NativePacketSizes.size())
		{
			return false;
		}

		return length == NativePacketSizes[index];
	}

} // namespace HOL
