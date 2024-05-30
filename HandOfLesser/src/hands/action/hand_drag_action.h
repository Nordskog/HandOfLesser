#pragma once

#include "base_action.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>
#include "src/openxr/XrUtils.h"

namespace HOL
{
	class HandDragAction : public BaseAction<Eigen::Vector2f>
	{
	public:
		HandDragAction(){};
		HandDragAction(HandSide side, XrHandJointEXT joint);
		static std::shared_ptr<HandDragAction> Create(HandSide side, XrHandJointEXT joint)
		{
			return std::make_shared<HandDragAction>(side, joint);
		}
		void onEvaluate(GestureData gestureData, ActionData actionData) override;

	private:
		Eigen::Vector3f mStartPosition = Eigen::Vector3f(0, 0, 0);
		HandSide mHandSide = HandSide::LeftHand;
		XrHandJointEXT mTargetJoint = XrHandJointEXT::XR_HAND_JOINT_THUMB_TIP_EXT;
		float mMultiplier = 5;
	};

} // namespace HOL