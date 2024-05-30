#include "button_action.h"

namespace HOL
{
	void ButtonAction::onEvaluate(GestureData gestureData, ActionData actionData)
	{
		this->mInputSink->submit(actionData.isDown);
	}
} // namespace HOL