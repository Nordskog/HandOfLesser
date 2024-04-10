#include "controller.h"
namespace HOL
{
	PoseLocationEuler getControllerBaseOffset(ControllerType type)
	{
		switch (type)
		{
			case ControllerType::OculusTouch_Airlink: {
				return {Eigen::Vector3f(0.076, -0.043, -0.107), Eigen::Vector3f(39.5f, 1, -91)};
			}

			case ControllerType::OculusTouch_VDXR: {
				return {Eigen::Vector3f(0.076, -0.043, -0.107), Eigen::Vector3f(39.5f, 1, -91)};
			}
		}
	}

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type)
	{
		switch (type)
		{
			case ControllerOffsetPreset::RoughyVRChatHand: {
				return {Eigen::Vector3f(0, 0.016, 0.002), Eigen::Vector3f(9, 0, 0)};
			}

			case ControllerOffsetPreset::ZERO: {
				return {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)};
			}
		}
	}

} // namespace HOL