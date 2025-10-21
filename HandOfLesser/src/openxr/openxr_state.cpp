#include "openxr_state.h"

namespace HOL::OpenXR
{
	const char* getOpenXrStateString(OpenXrState state)
	{
		switch (state)
		{
			case OpenXrState::Uninitialized:
				return "Uninitialized";
			case OpenXrState::Initialized:
				return "Initialized";
			case OpenXrState::Running:
				return "Running";
			case OpenXrState::Failed:
				return "Failed";
			case OpenXrState::Exited:
				return "Exited";
			default:
				return "Unknown";
		}
	}
} // namespace HOL::OpenXR