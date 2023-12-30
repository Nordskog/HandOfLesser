#pragma once

#include <HandOfLesserCommon.h>
#include "src/openxr/openxr_state.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	struct HandTransformDisplay
	{
		Eigen::Vector3f finalTranslationOffset;
		Eigen::Vector3f finalOrientationOffset; // In degrees
		HOL::PoseLocation rawPose;
		HOL::PoseLocation finalPose;
	};

	namespace display
	{
		extern HandTransformDisplay HandTransform[2];
		extern OpenXR::OpenXrState OpenXrInstanceState;
	}
} // namespace HOL