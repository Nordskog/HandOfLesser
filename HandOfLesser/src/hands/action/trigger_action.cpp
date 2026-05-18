#include "trigger_action.h"

namespace HOL
{
	void TriggerAction::onEvaluate(GestureData gestureData, ActionData actionData)
	{
		this->submitInput(InputType::Trigger, actionData.gestureValue);
		this->submitInput(InputType::Button, this->getButtonOutputState(actionData));
	}
} // namespace HOL
