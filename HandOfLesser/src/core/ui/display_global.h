#pragma once

#include <HandOfLesserCommon.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace HOL
{
	struct BodyTrackingDisplay
	{
		float confidence = 0;
	};

	struct HandTransformDisplay
	{
		bool active;
		bool positionValid;
		bool positionTracked;
		int trackedJointCount = 0;
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
		extern BodyTrackingDisplay BodyTracking;
	} // namespace display
} // namespace HOL
