#pragma once

#include <d3d11.h> // Why do you need this??
#include <memory>
#include "TrackedHand.h"

class HandTracking
{
	public:
		void init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session);
		void updateHands(xr::UniqueDynamicSpace& space, XrTime time);

	private:
		void initHands(xr::UniqueDynamicSession& session);
		std::unique_ptr<TrackedHand> mLeftHand;
		std::unique_ptr<TrackedHand> mRightHand;

};