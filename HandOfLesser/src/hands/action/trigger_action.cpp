#include "trigger_action.h"

namespace HOL
{
	void TriggerAction::onEvaluate(GestureData gestureData, ActionData actionData)
	{
		this->submitInput(InputType::Trigger, actionData.gestureValue);
		this->submitInput(InputType::Button, actionData.isDown ? 1 : 0);
	}
} // namespace HOL