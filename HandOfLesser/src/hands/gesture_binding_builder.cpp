#include "gesture_binding_builder.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

#include "src/hands/action/button_action.h"
#include "src/hands/action/hand_drag_action.h"
#include "src/hands/action/trigger_action.h"
#include "src/hands/gesture/aim_gesture.h"
#include "src/hands/gesture/chain_gesture.h"
#include "src/hands/gesture/combo_gesture.h"
#include "src/hands/gesture/finger_curl_gesture.h"
#include "src/hands/gesture/hold_gesture.h"
#include "src/hands/gesture/inverse_gesture.h"
#include "src/hands/gesture/look_at_gesture.h"
#include "src/hands/gesture/open_hand_pinch_gesture.h"
#include "src/hands/gesture/proximity_gesture.h"
#include "src/hands/input/settings_toggle_input.h"
#include "src/hands/input/steamvr_bool_input.h"
#include "src/hands/input/steamvr_float_input.h"
#include "src/core/settings_global.h"
#include "src/steamvr/input_wrapper.h"

namespace
{
	using HOL::FingerIndex;
	using HOL::FingerLittle;
	using HOL::FingerMiddle;
	using HOL::FingerRing;
	using HOL::FingerThumb;
	using HOL::HandSide;
	using HOL::HolSetting;
	using HOL::InputType;
	using HOL::settings::GestureBinding;
	using HOL::settings::GestureKind;
	using HOL::settings::GestureModifier;
	using HOL::settings::InputTarget;

	constexpr std::array<const char*, 4> FingerNames = {"Index", "Middle", "Ring", "Little"};

	constexpr std::array<HOL::GestureBindings::InputTargetOption, 11> InputTargetOptions = {{
		{InputTarget::Joystick, "Joystick"},
		{InputTarget::Grip, "Grip"},
		{InputTarget::Trigger, "Trigger"},
		{InputTarget::A, "A"},
		{InputTarget::B, "B"},
		{InputTarget::X, "X"},
		{InputTarget::Y, "Y"},
		{InputTarget::System, "System"},
		{InputTarget::Menu, "Menu"},
		{InputTarget::Thumbrest, "Thumbrest"},
		{InputTarget::Toggle_SteamVRInput, "Toggle SteamVR Input"},
	}};

	bool isAnalogTarget(InputTarget target)
	{
		return target == InputTarget::Grip || target == InputTarget::Trigger;
	}

	bool isToggleTarget(InputTarget target)
	{
		return target == InputTarget::Toggle_SteamVRInput;
	}

	bool isButtonTarget(InputTarget target)
	{
		return !isAnalogTarget(target) && !isToggleTarget(target)
			   && target != InputTarget::Joystick && target != InputTarget::None;
	}

	const HOL::SteamVR::InputWrapper& targetToWrapper(InputTarget target)
	{
		switch (target)
		{
			case InputTarget::Grip:
				return HOL::SteamVR::Input::Grip;
			case InputTarget::Trigger:
				return HOL::SteamVR::Input::Trigger;
			case InputTarget::A:
				return HOL::SteamVR::Input::A;
			case InputTarget::B:
				return HOL::SteamVR::Input::B;
			case InputTarget::X:
				return HOL::SteamVR::Input::X;
			case InputTarget::Y:
				return HOL::SteamVR::Input::Y;
			case InputTarget::System:
				return HOL::SteamVR::Input::System;
			case InputTarget::Menu:
				return HOL::SteamVR::Input::Menu;
			case InputTarget::Thumbrest:
				return HOL::SteamVR::Input::Thumbrest;
			default:
				break;
		}

		static HOL::SteamVR::InputWrapper fallback("");
		return fallback;
	}

	HolSetting targetToHolSetting(InputTarget target)
	{
		switch (target)
		{
			case InputTarget::Toggle_SteamVRInput:
				return HolSetting::SendSteamVRInput;
			default:
				return HolSetting::Default;
		}
	}

	std::array<HOL::FingerType, 3> getModifierFingers(HOL::FingerType pinchFinger)
	{
		std::array<HOL::FingerType, 3> modifierFingers{};
		size_t writeIndex = 0;

		for (HOL::FingerType finger : {FingerIndex, FingerMiddle, FingerRing, FingerLittle})
		{
			if (finger == pinchFinger)
			{
				continue;
			}

			modifierFingers[writeIndex++] = finger;
		}

		return modifierFingers;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildClosedHandModifier(
		HandSide side, HOL::FingerType pinchFinger)
	{
		auto curlCombo = HOL::Gesture::ComboGesture::Gesture::Create();
		curlCombo->parameters.holdUntilAllReleased = true;

		// For proximity gestures, "open" / "closed" should be based on the other three
		// non-thumb fingers, not always middle/ring/little.
		for (HOL::FingerType finger : getModifierFingers(pinchFinger))
		{
			auto curl = HOL::Gesture::FingerCurlGesture::Gesture::Create();
			curl->parameters.finger = finger;
			curl->parameters.side = side;
			curlCombo->addGesture(curl);
		}

		return curlCombo;
	}

	bool usesHold(const GestureBinding& binding)
	{
		return HOL::settings::hasGestureModifier(binding.modifiers, GestureModifier::Hold);
	}

	bool usesClosedHand(const GestureBinding& binding)
	{
		return HOL::settings::hasGestureModifier(binding.modifiers, GestureModifier::ClosedHand);
	}

	bool usesLookAtHand(const GestureBinding& binding)
	{
		return HOL::settings::hasGestureModifier(binding.modifiers, GestureModifier::LookingAtHand);
	}

	bool usesInFrontOfUser(const GestureBinding& binding)
	{
		return HOL::settings::hasGestureModifier(binding.modifiers, GestureModifier::InFrontOfUser);
	}

	bool supportsPressAndRelease(InputTarget target)
	{
		return isButtonTarget(target) || target == InputTarget::Trigger;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> wrapWithHold(
		const std::shared_ptr<HOL::Gesture::BaseGesture::Gesture>& gesture)
	{
		auto hold = HOL::Gesture::HoldGesture::Gesture::Create();
		hold->parameters.duration = std::chrono::milliseconds(HOL::Config.input.holdDurationMS);
		hold->setGesture(gesture);
		return hold;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> wrapWithLookAtHand(
		const std::shared_ptr<HOL::Gesture::BaseGesture::Gesture>& gesture, HandSide side)
	{
		auto lookAt = HOL::Gesture::LookAtGesture::Gesture::Create();
		lookAt->parameters.side = side;
		lookAt->parameters.fovDegrees = HOL::Config.input.lookAtFovDegrees;

		auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
		combo->parameters.holdUntilAllReleased = false;
		combo->parameters.valueMode = HOL::Gesture::ComboGesture::ValueMode::Product;
		combo->addGesture(gesture);
		combo->addGesture(lookAt);
		return combo;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> wrapWithInFrontOfUser(
		const std::shared_ptr<HOL::Gesture::BaseGesture::Gesture>& gesture, HandSide side)
	{
		auto inFront = HOL::Gesture::LookAtGesture::Gesture::Create();
		inFront->parameters.side = side;
		inFront->parameters.fovDegrees = HOL::Config.input.inFrontFovDegrees;

		auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
		combo->parameters.holdUntilAllReleased = false;
		combo->parameters.valueMode = HOL::Gesture::ComboGesture::ValueMode::Product;
		combo->addGesture(gesture);
		combo->addGesture(inFront);
		return combo;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildChainGesture(
		const GestureBinding& binding)
	{
		auto chain = HOL::Gesture::ChainGesture::Gesture::Create();
		chain->parameters.maxDelay
			= std::chrono::milliseconds(HOL::Config.input.chainGestureTimeoutMS);
		int chainLength = std::clamp(binding.chainLength, 1, GestureBinding::MaxChainLength);

		for (int i = 0; i < chainLength; i++)
		{
			HOL::FingerType finger = binding.chainFingers[i];
			auto pinch = HOL::Gesture::OpenHandPinchGesture::Gesture::Create();
			pinch->parameters.pinchFinger = finger;
			pinch->parameters.side = binding.side;
			pinch->setup();
			chain->addGesture(pinch);
		}

		return chain;
	}

	bool isRepeatedFingerChain(const GestureBinding& binding)
	{
		int chainLength = std::clamp(binding.chainLength, 1, GestureBinding::MaxChainLength);
		for (int i = 1; i < chainLength; i++)
		{
			if (binding.chainFingers[i] != binding.chainFingers[0])
			{
				return false;
			}
		}

		return true;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildOpenHandPinchGesture(
		HandSide side, HOL::FingerType finger)
	{
		auto pinch = HOL::Gesture::OpenHandPinchGesture::Gesture::Create();
		pinch->parameters.pinchFinger = finger;
		pinch->parameters.side = side;
		pinch->setup();
		return pinch;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildGripGesture(HandSide side)
	{
		auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
		combo->parameters.holdUntilAllReleased = true;

		for (HOL::FingerType finger : {FingerMiddle, FingerRing, FingerLittle})
		{
			auto curl = HOL::Gesture::FingerCurlGesture::Gesture::Create();
			curl->parameters.finger = finger;
			curl->parameters.side = side;
			combo->addGesture(curl);
		}

		return combo;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildSystemAimGesture(HandSide side)
	{
		auto aim = HOL::Gesture::AimGesture::Gesture::Create();
		aim->parameters.side = side;
		aim->parameters.flags = XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB;

		auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
		combo->parameters.holdUntilAllReleased = false;
		combo->addGesture(aim);

		// Require the non-thumb fingers to be open so the menu gesture does not overlap with
		// grab / trigger.
		for (HOL::FingerType finger : {FingerMiddle, FingerRing, FingerLittle})
		{
			auto curl = HOL::Gesture::FingerCurlGesture::Gesture::Create();
			curl->parameters.finger = finger;
			curl->parameters.side = side;

			auto inverse = HOL::Gesture::InverseGesture::Gesture::Create();
			inverse->setGesture(curl);
			inverse->setBinaryThreshold(1.0f);
			combo->addGesture(inverse);
		}

		return combo;
	}

	void addSinksForBinding(
		const std::shared_ptr<HOL::BaseAction>& action, const GestureBinding& binding)
	{
		if (binding.target == InputTarget::Joystick)
		{
			auto dragAction = std::dynamic_pointer_cast<HOL::HandDragAction>(action);
			if (!dragAction)
			{
				return;
			}

			auto xInput = HOL::SteamVRFloatInput::Create();
			xInput->setup(binding.side, HOL::SteamVR::Input::Joystick.x());
			dragAction->addSink(InputType::XAxis, xInput);

			auto yInput = HOL::SteamVRFloatInput::Create();
			yInput->setup(binding.side, HOL::SteamVR::Input::Joystick.y());
			dragAction->addSink(InputType::ZAxis, yInput);

			auto touchInput = HOL::SteamVRBoolInput::Create();
			touchInput->setup(binding.side, HOL::SteamVR::Input::Joystick.touch());
			dragAction->addSink(InputType::Touch, touchInput);
			return;
		}

		if (isAnalogTarget(binding.target))
		{
			const HOL::SteamVR::InputWrapper& wrapper = targetToWrapper(binding.target);

			auto boolInput = HOL::SteamVRBoolInput::Create();
			boolInput->setup(binding.side, wrapper.click());
			action->addSink(InputType::Button, boolInput);

			auto floatInput = HOL::SteamVRFloatInput::Create();
			floatInput->setup(binding.side, wrapper.value());

			// Touch-style controllers do not expose a real click action for grip/trigger.
			// Keep driving the value sink from the binary path so button activation stays
			// reliable across controller profiles.
			action->addSink(InputType::Button, floatInput);
			return;
		}

		if (isToggleTarget(binding.target))
		{
			auto toggle = HOL::SettingsToggleInput::Create();
			toggle->setup(targetToHolSetting(binding.target));
			action->addSink(InputType::Button, toggle);
			return;
		}

		if (isButtonTarget(binding.target))
		{
			const HOL::SteamVR::InputWrapper& wrapper = targetToWrapper(binding.target);
			auto boolInput = HOL::SteamVRBoolInput::Create();
			boolInput->setup(binding.side, wrapper.click());
			action->addSink(InputType::Button, boolInput);
		}
	}

	const char* fingerName(HOL::FingerType finger)
	{
		if (finger < FingerIndex || finger > FingerLittle)
		{
			return "Index";
		}

		return FingerNames[static_cast<size_t>(finger)];
	}
} // namespace

namespace HOL::GestureBindings
{
	const char* gestureKindName(GestureKind kind)
	{
		switch (kind)
		{
			case GestureKind::Proximity:
				return "Tap Finger";
			case GestureKind::Chain:
				return "Tap Sequence";
			case GestureKind::Grip:
				return "Grip";
			case GestureKind::SystemAim:
				return "System Gesture";
			default:
				return "(none)";
		}
	}

	const char* inputTargetName(InputTarget target)
	{
		for (const InputTargetOption& option : InputTargetOptions)
		{
			if (option.target == target)
			{
				return option.label;
			}
		}

		return "(none)";
	}

	std::string describeChainSequence(const GestureBinding& binding)
	{
		int chainLength = std::clamp(binding.chainLength, 1, GestureBinding::MaxChainLength);
		if (chainLength <= 0)
		{
			return "(none)";
		}

		bool repeatedFinger = true;
		for (int i = 1; i < chainLength; i++)
		{
			if (binding.chainFingers[i] != binding.chainFingers[0])
			{
				repeatedFinger = false;
				break;
			}
		}

		if (repeatedFinger)
		{
			std::string description = "Tap ";
			description += fingerName(binding.chainFingers[0]);
			description += " x";
			description += std::to_string(chainLength);
			return description;
		}

		std::string description = "Tap ";
		for (int i = 0; i < chainLength; i++)
		{
			if (i > 0)
			{
				description += " -> ";
			}
			description += fingerName(binding.chainFingers[i]);
		}
		return description;
	}

	std::string describeBinding(const GestureBinding& binding)
	{
		switch (binding.kind)
		{
			case GestureKind::Proximity:
			{
				std::string description = "Tap ";
				description += fingerName(binding.proximityFinger);
				description += " Finger";

				if (usesClosedHand(binding))
				{
					description += " (Closed Hand)";
				}

				if (usesHold(binding))
				{
					description += " (Hold)";
				}

				if (usesLookAtHand(binding))
				{
					description += " (Look At Hand)";
				}

				if (usesInFrontOfUser(binding))
				{
					description += " (In Front)";
				}

				if (binding.pressAndRelease)
				{
					description += " (Press and Release)";
				}

				return description;
			}

			case GestureKind::Chain:
			{
				std::string description = describeChainSequence(binding);
				if (usesHold(binding))
				{
					description += " (Hold)";
				}

				if (usesLookAtHand(binding))
				{
					description += " (Look At Hand)";
				}
				if (usesInFrontOfUser(binding))
				{
					description += " (In Front)";
				}
				if (binding.pressAndRelease)
				{
					description += " (Press and Release)";
				}
				return description;
			}

			case GestureKind::Grip:
			case GestureKind::SystemAim:
			{
				std::string description = gestureKindName(binding.kind);
				if (usesHold(binding))
				{
					description += " (Hold)";
				}

				if (usesLookAtHand(binding))
				{
					description += " (Look At Hand)";
				}
				if (usesInFrontOfUser(binding))
				{
					description += " (In Front)";
				}
				if (binding.pressAndRelease)
				{
					description += " (Press and Release)";
				}
				return description;
			}

			default:
				return "(none)";
		}
	}

	std::span<const InputTargetOption> inputTargetOptions()
	{
		return InputTargetOptions;
	}

	bool isGestureTargetCompatible(GestureKind kind, InputTarget target)
	{
		switch (kind)
		{
			case GestureKind::Proximity:
				return target != InputTarget::None;

			case GestureKind::Chain:
				return isButtonTarget(target) || isToggleTarget(target);

			case GestureKind::Grip:
				return isAnalogTarget(target) || isButtonTarget(target)
					   || isToggleTarget(target);

			case GestureKind::SystemAim:
				return target == InputTarget::System;

			default:
				return false;
		}
	}

	InputTarget firstCompatibleInputTarget(GestureKind kind)
	{
		for (const InputTargetOption& option : inputTargetOptions())
		{
			if (isGestureTargetCompatible(kind, option.target))
			{
				return option.target;
			}
		}

		return InputTarget::None;
	}

	std::shared_ptr<HOL::BaseAction> buildAction(const GestureBinding& binding)
	{
		if (!binding.enabled || binding.kind == GestureKind::None
			|| binding.target == InputTarget::None)
		{
			return nullptr;
		}

		if (!isGestureTargetCompatible(binding.kind, binding.target))
		{
			std::cerr << "GestureBindings: incompatible binding ("
					  << gestureKindName(binding.kind) << " -> "
					  << inputTargetName(binding.target) << ")\n";
			return nullptr;
		}

		std::shared_ptr<HOL::BaseAction> action;

		if (binding.target == InputTarget::Joystick)
		{
			auto dragAction = HOL::HandDragAction::Create();

			// Keep the drag anchor on the thumb tip like the original hard-coded binding.
			dragAction->setup(binding.side, XR_HAND_JOINT_THUMB_TIP_EXT);

			auto triggerGesture = HOL::Gesture::OpenHandPinchGesture::Gesture::Create();
			triggerGesture->parameters.pinchFinger = binding.proximityFinger;
			triggerGesture->parameters.side = binding.side;
			triggerGesture->setup();

			auto holdGesture = HOL::Gesture::ProximityGesture::Create();
			holdGesture->setup(binding.proximityFinger, binding.side);
			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> finalHoldGesture = holdGesture;
			if (usesLookAtHand(binding))
			{
				finalHoldGesture = wrapWithLookAtHand(finalHoldGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				finalHoldGesture = wrapWithInFrontOfUser(finalHoldGesture, binding.side);
			}

			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> finalTriggerGesture = triggerGesture;
			if (usesLookAtHand(binding))
			{
				finalTriggerGesture = wrapWithLookAtHand(finalTriggerGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				finalTriggerGesture = wrapWithInFrontOfUser(finalTriggerGesture, binding.side);
			}
			if (usesHold(binding))
			{
				finalTriggerGesture = wrapWithHold(finalTriggerGesture);
			}

			dragAction->setTriggerGesture(finalTriggerGesture);
			dragAction->setHoldGesture(finalHoldGesture);
			dragAction->getParameters().releaseThreshold = 0.8f;
			action = dragAction;
		}
		else if (binding.kind == GestureKind::Proximity)
		{
			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> triggerGesture
				= buildOpenHandPinchGesture(binding.side, binding.proximityFinger);

			if (usesClosedHand(binding)
				&& binding.proximityFinger == HOL::FingerIndex)
			{
				auto proximity = HOL::Gesture::ProximityGesture::Create();
				proximity->setup(binding.proximityFinger, binding.side, FingerThumb, binding.side);

				auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
				combo->parameters.holdUntilAllReleased = false;
				combo->parameters.valueMode = HOL::Gesture::ComboGesture::ValueMode::Product;
				combo->addGesture(proximity);
				combo->addGesture(
					buildClosedHandModifier(binding.side, binding.proximityFinger));
				triggerGesture = combo;
			}

			if (isAnalogTarget(binding.target))
			{
				auto triggerAction = HOL::TriggerAction::Create();

				// Grab used a softer release threshold before the configurable refactor; keep
				// that behavior for any grip-like analog binding without changing trigger.
				if (binding.target == InputTarget::Grip)
				{
					triggerAction->getParameters().releaseThreshold = 0.8f;
				}

				action = triggerAction;
			}
			else
			{
				action = HOL::ButtonAction::Create();
			}

			if (supportsPressAndRelease(binding.target))
			{
				action->getParameters().pressAndRelease = binding.pressAndRelease;
			}

			if (usesLookAtHand(binding))
			{
				triggerGesture = wrapWithLookAtHand(triggerGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				triggerGesture = wrapWithInFrontOfUser(triggerGesture, binding.side);
			}

			if (usesHold(binding))
			{
				triggerGesture = wrapWithHold(triggerGesture);
			}

			action->setTriggerGesture(triggerGesture);
		}
		else if (binding.kind == GestureKind::Chain)
		{
			action = HOL::ButtonAction::Create();
			action->getParameters().pressAndRelease = binding.pressAndRelease;
			if (isRepeatedFingerChain(binding))
			{
				// When the same pinch is tapped repeatedly, the thumb often only lifts slightly
				// between taps. Tighten release hysteresis so the action does not remain held
				// between intended taps.
				action->getParameters().releaseThreshold = 0.999f;
			}
			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> triggerGesture
				= buildChainGesture(binding);
			if (usesLookAtHand(binding))
			{
				triggerGesture = wrapWithLookAtHand(triggerGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				triggerGesture = wrapWithInFrontOfUser(triggerGesture, binding.side);
			}
			if (usesHold(binding))
			{
				triggerGesture = wrapWithHold(triggerGesture);
			}
			action->setTriggerGesture(triggerGesture);
		}
		else if (binding.kind == GestureKind::Grip)
		{
			auto gripGesture = buildGripGesture(binding.side);
			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> triggerGesture = gripGesture;
			if (usesLookAtHand(binding))
			{
				triggerGesture = wrapWithLookAtHand(triggerGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				triggerGesture = wrapWithInFrontOfUser(triggerGesture, binding.side);
			}
			if (usesHold(binding))
			{
				triggerGesture = wrapWithHold(triggerGesture);
			}

			if (isAnalogTarget(binding.target))
			{
				auto triggerAction = HOL::TriggerAction::Create();
				if (binding.target == InputTarget::Grip)
				{
					triggerAction->getParameters().releaseThreshold = 0.8f;
				}

				action = triggerAction;
			}
			else
			{
				action = HOL::ButtonAction::Create();
			}

			if (supportsPressAndRelease(binding.target))
			{
				action->getParameters().pressAndRelease = binding.pressAndRelease;
			}

			action->setTriggerGesture(triggerGesture);
		}
		else if (binding.kind == GestureKind::SystemAim)
		{
			action = HOL::ButtonAction::Create();
			action->getParameters().pressAndRelease = binding.pressAndRelease;
			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> triggerGesture
				= buildSystemAimGesture(binding.side);
			if (usesLookAtHand(binding))
			{
				triggerGesture = wrapWithLookAtHand(triggerGesture, binding.side);
			}
			if (usesInFrontOfUser(binding))
			{
				triggerGesture = wrapWithInFrontOfUser(triggerGesture, binding.side);
			}
			if (usesHold(binding))
			{
				triggerGesture = wrapWithHold(triggerGesture);
			}
			action->setTriggerGesture(triggerGesture);
		}

		if (action)
		{
			addSinksForBinding(action, binding);
		}

		return action;
	}
} // namespace HOL::GestureBindings
