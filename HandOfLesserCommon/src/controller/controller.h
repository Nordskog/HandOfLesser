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
		ValveIndexKnuckles,
		ControllerType_MAX
	};

	PoseLocationEuler getDefaultControllerOffset(ControllerType type);

}