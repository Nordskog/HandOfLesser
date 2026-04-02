#pragma once

#include <HandOfLesserCommon.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
namespace HOL
{
	class HandPose
	{
	public:
		FingerBend fingers[FingerType::FingerType_MAX];

		bool active;
		bool poseValid;
		bool poseTracked;
		bool poseStale;

		// Raw palm-space values from the runtime.
		// These are used for SteamVR controller pose generation so the driver can apply the
		// controller offset through DriverFromHead instead of baking it into the sensor pose.
		HOL::PoseLocation palmLocation;
		HOL::PoseVelocity palmVelocity;

		// Final controller-aligned pose after base + user offsets have been applied.
		HOL::PoseLocation controllerLocation;
		HOL::PoseVelocity controllerVelocity;

		// offset to match Index tracker location
		HOL::PoseLocation trackerLocation;
	};
} // namespace HOL
