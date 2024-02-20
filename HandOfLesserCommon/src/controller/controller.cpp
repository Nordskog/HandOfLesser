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
					Eigen::Vector3f(0.021, 0, -0.114), 
					Eigen::Vector3f(63, 0, -99)
				};
			}

			case ControllerType::ValveIndexKnuckles: 
			{
				return 
				{
					Eigen::Vector3f(0.021, 0, -0.114), 
					Eigen::Vector3f(63, 0, -99)
				};
			}
		}
	}
}