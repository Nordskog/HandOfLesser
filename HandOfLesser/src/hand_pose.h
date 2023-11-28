#pragma once

#include <HandOfLesserCommon.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
namespace HOL
{
	class HandPose
	{
	public:
		FingerBend fingers[FingerType::FINGER_MAX_COUNT];

		bool poseValid;

		// raw values
		HOL::PoseLocation palmLocation;
		HOL::PoseVelocity palmVelocity; // Does velocity need to be modified for tracker?

		// offset to match Index tracker location
		HOL::PoseLocation trackerLocation;
	};
} // namespace HOL