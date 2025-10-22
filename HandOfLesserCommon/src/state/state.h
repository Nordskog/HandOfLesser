#pragma once

#include "../openxr/openxr_state.h"
#include <cstddef>
#include <cstring>

namespace HOL::state
{
	struct TrackingState
	{
		bool isMultimodalEnabled = false;
		bool isHighFidelityEnabled = false;
	};

	struct RuntimeState
	{
		bool isVDXR = false;
		bool isOVR = false;
		HOL::OpenXR::OpenXrState openxrState = HOL::OpenXR::OpenXrState::Uninitialized;
		char runtimeName[128] = {};
	};
} // namespace HOL::state
