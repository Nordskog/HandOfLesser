#pragma once

#include "hol_utils.h"

namespace HOL
{
	std::chrono::milliseconds timeSince(std::chrono::steady_clock::time_point time)
	{
		auto currentTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - time);
	}
} // namespace HOL
