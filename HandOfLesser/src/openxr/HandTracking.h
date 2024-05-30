#pragma once

#include <d3d11.h> // Why do you need this??
#include <memory>
#include "openxr_hand.h"
#include "src/hands/gesture/open_hand_pinch_gesture.h"
#include "src/hands/action/hand_drag_action.h"

#include "src/vrchat/vrchat_input.h";

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
		void drawHands();

	private:
		void initHands(xr::UniqueDynamicSession& session);
		void initGestures();
		void updateSimpleGestures();
		void updateGestures();
		OpenXRHand& getHand(HOL::HandSide side);
		OpenXRHand mLeftHand;
		OpenXRHand mRightHand;

		std::vector<std::shared_ptr<ProtoAction>> mActions;
	};
} // namespace HOL::OpenXR
