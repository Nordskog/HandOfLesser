#pragma once

namespace HOL::OpenXR
{
	enum class OpenXrState
	{
		Uninitialized = 0,
		Initialized,
		Running,
		Failed,
		Exited
	};

	const char* getOpenXrStateString(OpenXrState state);
} // namespace HOL::OpenXR
