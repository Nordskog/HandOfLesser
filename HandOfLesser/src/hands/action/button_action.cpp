#include "button_action.h"

namespace HOL
{
	void ButtonAction::onEvaluate(GestureData gestureData, ActionData actionData)
	{
		this->submitInput(InputType::Button, actionData.isDown);
	}
} // namespace HOL