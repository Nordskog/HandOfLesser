#include "controller.h"
namespace HOL
{
	PoseLocationEuler getControllerBaseOffset(ControllerType type)
	{
		switch (type)
		{
			case ControllerType::NONE: {
				return {Eigen::Vector3f(0,0,0), Eigen::Vector3f(0,0,0)};
			}

			case ControllerType::ValveIndexKnucles: {
				return {Eigen::Vector3f(0.076, -0.043, -0.107),
						Eigen::Vector3f(0.176f, -39.510f, -91.296f)};
			}

			case ControllerType::OculusTouch_Airlink: {
				return {Eigen::Vector3f(0.076, -0.043, -0.107),
						Eigen::Vector3f(0.176f, -39.510f, -91.296f)};
			}

			case ControllerType::OculusTouch_VDXR: {
				return {Eigen::Vector3f(0.076, -0.043, -0.107),
						Eigen::Vector3f(0.176f, -39.510f, -91.296f)};
			}
		}
	}

	PoseLocationEuler getControllerOffsetPreset(ControllerOffsetPreset type)
	{
		switch (type)
		{
			case ControllerOffsetPreset::RoughyVRChatHand: {
				return {Eigen::Vector3f(-0.019, -0.022, 0.003),
						Eigen::Vector3f(169.963f, 175.076f, -179.130f)};
			}

			case ControllerOffsetPreset::ZERO: {
				return {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)};
			}
		}
	}

} // namespace HOL