#pragma once

#include "base_action.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>
#include "src/openxr/XrUtils.h"

namespace HOL
{
	class TriggerAction : public BaseAction
	{
	public:
		TriggerAction() : BaseAction({InputType::Trigger, InputType::Button, InputType::Touch}){};
		static std::shared_ptr<TriggerAction> Create()
		{
			return std::make_shared<TriggerAction>();
		}
		void onEvaluate(GestureData gestureData, ActionData actionData) override;

	private:
	};

} // namespace HOL