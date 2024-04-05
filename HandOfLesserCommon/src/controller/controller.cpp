#include "controller.h"
namespace HOL
{
	PoseLocationEuler getControllerBaseOffset(ControllerType type)
	{
		switch (type)
		{
			case ControllerType::OculusTouch_Airlink: {
				return {Eigen::Vector3f(0.044, -0.009, -0.132), Eigen::Vector3f(40, 1, -91)};
			}

			case ControllerType::OculusTouch_VDXR: {
				return {Eigen::Vector3f(0.044, -0.009, -0.132), Eigen::Vector3f(40, 1, -91)};
			}
		}
	}

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type)
	{
		switch (type)
		{
			case ControllerOffsetPreset::RoughyVRChatHand: {
				return {Eigen::Vector3f(-0.011, -0.004, 0.007), Eigen::Vector3f(8, 0, 0)};
			}

			case ControllerOffsetPreset::ZERO: {
				return {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)};
			}
		}
	}

} // namespace HOL