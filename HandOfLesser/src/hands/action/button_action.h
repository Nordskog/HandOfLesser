#pragma once

#include "base_action.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>
#include "src/openxr/XrUtils.h"

namespace HOL
{
	class ButtonAction : public BaseAction
	{
	public:
		ButtonAction() : BaseAction({InputType::Button}){};
		static std::shared_ptr<ButtonAction> Create()
		{
			return std::make_shared<ButtonAction>();
		}
		void onEvaluate(GestureData gestureData, ActionData actionData) override;

	private:
	};

} // namespace HOL