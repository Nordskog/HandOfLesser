#pragma once

namespace HOL::OpenXR
{
	enum OpenXrState
	{
		Uninitialized,
		Initialized,
		Running,
		Failed,
		Exited,
	};

	const char* getOpenXrStateString(OpenXrState state);
} // namespace HOL::OpenXR
