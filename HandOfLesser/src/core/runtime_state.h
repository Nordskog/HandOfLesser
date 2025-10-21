#pragma once

#include <cstring>
#include "../openxr/openxr_state.h"

namespace HOL::state
{
	struct RuntimeState
	{
		bool isVDXR = false;
		bool isOVR = false;
		OpenXR::OpenXrState openxrState = OpenXR::OpenXrState::Uninitialized;
		char runtimeName[128] = {};
	};
} // namespace HOL::state
