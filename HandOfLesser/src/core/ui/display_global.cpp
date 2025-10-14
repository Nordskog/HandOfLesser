#pragma once

#include "display_global.h"

namespace HOL::display
{
	HandTransformDisplay HandTransform[2];
	FingerTrackingDisplay FingerTracking[2];
	BodyTrackingDisplay BodyTracking;

	OpenXR::OpenXrState OpenXrInstanceState = OpenXR::OpenXrState::Uninitialized;
	std::string OpenXrRuntimeName = "Unknown";
	bool IsVDXR = false;
	bool isOVR = false;

} // namespace HOL::display
