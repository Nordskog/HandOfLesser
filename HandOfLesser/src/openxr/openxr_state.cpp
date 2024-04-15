#include "openxr_state.h"

namespace HOL::OpenXR
{
	const char* getOpenXrStateString(OpenXrState state)
	{
		switch (state)
		{
			case HOL::OpenXR::Uninitialized:
				return "Uninitialized";
			case HOL::OpenXR::Initialized:
				return "Initialized";
			case HOL::OpenXR::Running:
				return "Running";
			case HOL::OpenXR::Failed:
				return "Failed";
			case HOL::OpenXR::Exited:
				return "Exited";
			default:
				return "Unknown";
		}
	}
}