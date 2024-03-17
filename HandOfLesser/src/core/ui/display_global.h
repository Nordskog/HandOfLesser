#pragma once

#include <HandOfLesserCommon.h>
#include "src/openxr/openxr_state.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	struct HandTransformDisplay
	{
		bool active;
		bool positionValid;
		bool positionTracked;
		Eigen::Vector3f finalTranslationOffset;
		Eigen::Vector3f finalOrientationOffset; // In degrees
		PoseLocation rawPose;
		PoseLocation finalPose;
	};

	struct FingerTrackingDisplay
	{
		FingerBend rawBend[FingerType::FingerType_MAX];
		FingerBend humanoidBend[FingerType::FingerType_MAX];
		FingerBend packedBend[FingerType::FingerType_MAX]; // reusing for int values
	};

	namespace display
	{
		extern FingerTrackingDisplay FingerTracking[2];
		extern HandTransformDisplay HandTransform[2];

		extern OpenXR::OpenXrState OpenXrInstanceState;
		extern std::string OpenXrRuntimeName;
		extern bool IsVDXR;
	} // namespace display
} // namespace HOL