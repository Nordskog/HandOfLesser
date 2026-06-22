#pragma once

#include "display_global.h"

namespace HOL::display
{
	HandTransformDisplay HandTransform[2];
	FingerTrackingDisplay FingerTracking[2];
	BodyTrackingDisplay BodyTracking;
	DriverStatusDisplay DriverStatus;

	void updateTrackingRate(std::chrono::steady_clock::time_point& lastUpdateTime,
							std::atomic<float>& updateRateMS)
	{
		auto now = std::chrono::steady_clock::now();
		if (lastUpdateTime != std::chrono::steady_clock::time_point{})
		{
			updateRateMS.store(
				std::chrono::duration<float, std::milli>(now - lastUpdateTime).count());
		}
		lastUpdateTime = now;
	}
} // namespace HOL::display
