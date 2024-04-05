#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <src/hand/hand.h>

namespace HOL
{
	enum ControllerType
	{
		OculusTouch_Airlink,
		OculusTouch_VDXR, 
		ControllerType_MAX
	};

	// TODO: need to be able to save these
	enum ControllerOffsetPreset
	{
		ZERO,
		RoughyVRChatHand,
		ControllerOffsetPreset_MAX
	};

	PoseLocationEuler getControllerBaseOffset(ControllerType type);

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type);
}