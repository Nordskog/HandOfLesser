#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <src/hand/hand.h>

#include <d3d11.h>
#include <openxr/openxr.h>

namespace HOL
{
	enum ControllerType
	{
		NONE,
		ValveIndexKnucles,
		OculusTouch_Airlink,
		OculusTouch_VDXR,
		ControllerType_MAX
	};

	// TODO: need to be able to save these
	enum ControllerOffsetPreset
	{
		ZERO,
		RoughyVRChatHand,
		RoughyVRChatHand2,
		ControllerOffsetPreset_MAX
	};

	enum ControllerMode
	{
		NoControllerMode,
		EmulateControllerMode,
		HookedControllerMode,
		OffsetControllerMode,
		ControllerMode_MAX
	};

	enum class BodyTrackerRole : int
	{
		Hips = 0,
		Chest = 1,
		LeftUpperArm = 2,
		LeftLowerArm = 3,
		RightUpperArm = 4,
		RightLowerArm = 5,
		TrackerRole_MAX = 6
	};

	PoseLocationEuler getControllerBaseOffset(ControllerType type);

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type);

	// Body tracker helper functions
	const char* bodyTrackerRoleToString(BodyTrackerRole role);
	const char* bodyTrackerRoleToSerial(BodyTrackerRole role);
	XrBodyJointFB bodyTrackerRoleToJoint(BodyTrackerRole role);
	const char* bodyTrackerRoleToTrackerRoleString(BodyTrackerRole role);
} // namespace HOL
