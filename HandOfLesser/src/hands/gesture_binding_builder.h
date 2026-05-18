#pragma once

#include <memory>
#include <span>
#include <string>
#include <HandOfLesserCommon.h>
#include "src/hands/action/base_action.h"
#include "src/settings/settings.h"

namespace HOL::GestureBindings
{
	struct InputTargetOption
	{
		settings::InputTarget target;
		const char* label;
	};

	// Builds a BaseAction from a GestureBinding configuration.
	// Returns nullptr if the binding is disabled or has invalid configuration.
	std::shared_ptr<BaseAction> buildAction(const settings::GestureBinding& binding);

	// Shared UI/builder helpers. Keep gesture metadata centralized so the editor and
	// runtime agree on labels and compatibility rules.
	const char* gestureKindName(settings::GestureKind kind);
	const char* inputTargetName(settings::InputTarget target);
	const char* chainDirectionName(settings::ChainDirection direction);
	std::string describeBinding(const settings::GestureBinding& binding);
	std::span<const InputTargetOption> inputTargetOptions();

	bool isGestureTargetCompatible(settings::GestureKind kind, settings::InputTarget target);
	settings::InputTarget firstCompatibleInputTarget(settings::GestureKind kind);
} // namespace HOL::GestureBindings
