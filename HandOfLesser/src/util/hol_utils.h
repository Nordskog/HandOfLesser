#pragma once

#include <chrono>

namespace HOL
{
	std::chrono::milliseconds timeSince(std::chrono::steady_clock::time_point time);
}
