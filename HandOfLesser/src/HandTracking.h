#pragma once

#include <d3d11.h> // Why do you need this??
#include <memory>
#include "TrackedHand.h"

class HandTracking
{
	public:
		void init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session);
		void updateHands(xr::UniqueDynamicSpace& space, XrTime time);
		void updateInputs();
		HOL::HandTransformPacket getTransformPacket(XrHandEXT side);
		HOL::ControllerInputPacket getInputPacket(XrHandEXT side);

	private:
		void initHands(xr::UniqueDynamicSession& session);
		void updateSimpleGestures();
		std::unique_ptr<TrackedHand> mLeftHand;
		std::unique_ptr<TrackedHand> mRightHand;

};