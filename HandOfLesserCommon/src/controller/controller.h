#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <src/hand/hand.h>

namespace HOL
{
	enum ControllerType
	{
		OculusTouch, 
		ValveIndexKnuckles,
		ControllerType_MAX
	};

	PoseLocationEuler getDefaultControllerOffset(ControllerType type);

}