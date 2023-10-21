#pragma once

#include <HandOfLesserCommon.h>
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace HOL
{
	struct HandTransformDisplay
	{
		HOL::PoseLocation finalOffset;
		HOL::PoseLocation rawPose;
		HOL::PoseLocation finalPose;
	};

	namespace display
	{
		extern HandTransformDisplay HandTransform[2];
	}
}