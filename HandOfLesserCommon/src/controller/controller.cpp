#include "controller.h"
namespace HOL
{
	PoseLocationEuler getDefaultControllerOffset(ControllerType type)
	{
		switch (type)
		{
			case ControllerType::OculusTouch: {
				return 
				{
					Eigen::Vector3f(0.011, -0.015, -0.013), 
					Eigen::Vector3f(-12, 0, 0)
				};
			}

			case ControllerType::ValveIndexKnuckles: 
			{
				return 
				{
					Eigen::Vector3f(0.011, -0.015, -0.013), 
					Eigen::Vector3f(-12, 0, 0)
				};
			}
		}
	}
}