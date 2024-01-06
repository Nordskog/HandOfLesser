#pragma once

#include <d3d11.h> // Why do you need this??
#include <memory>
#include "openxr_hand.h"

namespace HOL::OpenXR
{
	class HandTracking
	{
	public:
		void init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session);
		void updateHands(xr::UniqueDynamicSpace& space, XrTime time);
		void updateInputs();
		HOL::HandTransformPacket getTransformPacket(HOL::HandSide side);
		HOL::ControllerInputPacket getInputPacket(HOL::HandSide side);
		HOL::HandPose& getHandPose(HOL::HandSide side);

	private:
		void initHands(xr::UniqueDynamicSession& session);
		void updateSimpleGestures();
		OpenXRHand& getHand(HOL::HandSide side);
		OpenXRHand mLeftHand;
		OpenXRHand mRightHand;
	};
} // namespace HOL::OpenXR
