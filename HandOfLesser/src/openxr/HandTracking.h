#pragma once

#include <d3d11.h> // Why do you need this??
#include <atomic>
#include <memory>
#include "openxr_hand.h"
#include "openxr_body.h"
#include <HandOfLesserCommon.h>
#include "src/hands/gesture/open_hand_pinch_gesture.h"
#include "src/hands/action/hand_drag_action.h"

#include "src/vrchat/vrchat_input.h"

#include "src/hands/gesture/combo_gesture.h"

namespace HOL::OpenXR
{
	class HandTracking
	{
	public:
		void init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session);
		void updateHands(xr::UniqueDynamicSpace& space, XrTime time, OpenXRBody& bodyTracker);
		void updateInputs();
		HOL::HandTransformPayload getTransformPayload(HOL::HandSide side);
		HOL::HandPose& getHandPose(HOL::HandSide side);
		void drawHands();
		OpenXRHand* getHand(HOL::HandSide side);

		void rebuildActions();
		std::shared_ptr<BaseAction> getActionForBindingIndex(size_t bindingIndex) const;

	private:
		struct ActionSet
		{
			std::vector<std::shared_ptr<BaseAction>> actions;
			std::vector<std::shared_ptr<BaseAction>> actionsByBindingIndex;
		};

		void initHands(xr::UniqueDynamicSession& session);
		void submitLegacyFingerCurl();
		void updateSimpleGestures();
		OpenXRHand mLeftHand;
		OpenXRHand mRightHand;

		std::shared_ptr<const ActionSet> mActionSet = std::make_shared<ActionSet>();
	};
} // namespace HOL::OpenXR
