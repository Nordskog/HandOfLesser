#pragma once

#include "display_global.h"

namespace HOL::display
{
	HandTransformDisplay HandTransform[2];
	FingerTrackingDisplay FingerTracking[2];

	OpenXR::OpenXrState OpenXrInstanceState = OpenXR::OpenXrState::Uninitialized;
	std::string OpenXrRuntimeName = "Unknown";
	bool IsVDXR = false;

} // namespace HOL::display
