#include "ui_gesture_debug.h"

#include "imgui.h"
#include "src/hands/action/base_action.h"

namespace
{
	void drawGestureValueTree(float scale,
							  const std::shared_ptr<HOL::Gesture::BaseGesture::Gesture>& gesture,
							  const char* label,
							  int depth)
	{
		if (!gesture)
		{
			return;
		}

		if (depth > 0)
		{
			ImGui::Indent(scale * 14.0f);
		}

		ImGui::Text("%s%s%s: %.3f",
					label ? label : "",
					label ? " " : "",
					gesture->name.c_str(),
					gesture->lastValue);

		for (auto& subGesture : gesture->getSubGestures())
		{
			drawGestureValueTree(scale, subGesture, nullptr, depth + 1);
		}

		if (depth > 0)
		{
			ImGui::Unindent(scale * 14.0f);
		}
	}
} // namespace

void HOL::UiGestureDebug::drawActionDebug(float scale, const std::shared_ptr<BaseAction>& action)
{
	if (!action)
	{
		ImGui::TextDisabled("No live action built for this binding.");
		return;
	}

	const ActionData& actionData = action->getActionData();
	ImGui::Text("Action: value=%.3f  touch=%s  down=%s  onDown=%s  onUp=%s",
				actionData.gestureValue,
				actionData.isTouch ? "true" : "false",
				actionData.isDown ? "true" : "false",
				actionData.onDown ? "true" : "false",
				actionData.onUp ? "true" : "false");

	drawGestureValueTree(scale, action->getTriggerGesture(), "Trigger", 0);
	if (action->usesHoldGesture())
	{
		drawGestureValueTree(scale, action->getHoldGesture(), "Hold", 0);
	}
	if (action->getTapGesture())
	{
		drawGestureValueTree(scale, action->getTapGesture(), "Tap", 0);
	}
}
