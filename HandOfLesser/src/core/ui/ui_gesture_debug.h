#pragma once

#include <memory>

namespace HOL
{
	class BaseAction;

	class UiGestureDebug
	{
	public:
		static void drawActionDebug(float scale, const std::shared_ptr<BaseAction>& action);
	};
} // namespace HOL
