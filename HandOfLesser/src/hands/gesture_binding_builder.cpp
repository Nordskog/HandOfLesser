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
#include "src/hands/gesture/inverse_gesture.h"
#include "src/hands/gesture/open_hand_pinch_gesture.h"
#include "src/hands/gesture/proximity_gesture.h"
#include "src/hands/input/settings_toggle_input.h"
#include "src/hands/input/steamvr_bool_input.h"
#include "src/hands/input/steamvr_float_input.h"
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
	using HOL::settings::ChainDirection;
	using HOL::settings::GestureBinding;
	using HOL::settings::GestureKind;
	using HOL::settings::GripModifier;
	using HOL::settings::InputTarget;

	constexpr std::array<const char*, 4> FingerNames = {"Index", "Middle", "Ring", "Little"};

	constexpr std::array<HOL::GestureBindings::InputTargetOption, 12> InputTargetOptions = {{
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
		{InputTarget::Toggle_OscInput, "Toggle OSC Input"},
	}};

	bool isAnalogTarget(InputTarget target)
	{
		return target == InputTarget::Grip || target == InputTarget::Trigger;
	}

	bool isToggleTarget(InputTarget target)
	{
		return target == InputTarget::Toggle_SteamVRInput
			   || target == InputTarget::Toggle_OscInput;
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
			case InputTarget::Toggle_OscInput:
				return HolSetting::SendOscInput;
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

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildGripModifier(
		GripModifier modifier, HandSide side, HOL::FingerType pinchFinger)
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

		if (modifier == GripModifier::Closed)
		{
			return curlCombo;
		}

		auto inverse = HOL::Gesture::InverseGesture::Gesture::Create();
		inverse->setGesture(curlCombo);
		inverse->setBinaryThreshold(1.0f);
		return inverse;
	}

	std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> buildChainGesture(
		HandSide side, ChainDirection direction)
	{
		auto chain = HOL::Gesture::ChainGesture::Gesture::Create();
		std::array<HOL::FingerType, 4> fingers = {
			FingerIndex, FingerMiddle, FingerRing, FingerLittle};

		if (direction == ChainDirection::Descending)
		{
			std::reverse(fingers.begin(), fingers.end());
		}

		for (HOL::FingerType finger : fingers)
		{
			auto pinch = HOL::Gesture::OpenHandPinchGesture::Gesture::Create();
			pinch->parameters.pinchFinger = finger;
			pinch->parameters.side = side;
			pinch->setup();
			chain->addGesture(pinch);
		}

		return chain;
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
				return "Tap All Fingers";
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

	const char* gripModifierName(GripModifier modifier)
	{
		switch (modifier)
		{
			case GripModifier::None:
				return "None";
			case GripModifier::Open:
				return "Open Hand";
			case GripModifier::Closed:
				return "Closed Hand";
			default:
				return "(unknown)";
		}
	}

	const char* chainDirectionName(ChainDirection direction)
	{
		switch (direction)
		{
			case ChainDirection::Ascending:
				return "Index -> Pinky";
			case ChainDirection::Descending:
				return "Pinky -> Index";
			default:
				return "(unknown)";
		}
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

				if (binding.modifier != GripModifier::None)
				{
					description += " (";
					description += gripModifierName(binding.modifier);
					description += ")";
				}

				return description;
			}

			case GestureKind::Chain:
				return std::string("Tap ") + chainDirectionName(binding.chainDirection);

			case GestureKind::Grip:
			case GestureKind::SystemAim:
				return gestureKindName(binding.kind);

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

		if (isGestureTargetCompatible(kind, InputTarget::Toggle_OscInput))
		{
			return InputTarget::Toggle_OscInput;
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

			dragAction->setTriggerGesture(triggerGesture);
			dragAction->setHoldGesture(holdGesture);
			dragAction->getParameters().releaseThreshold = 0.8f;
			action = dragAction;
		}
		else if (binding.kind == GestureKind::Proximity)
		{
			auto proximity = HOL::Gesture::ProximityGesture::Create();
			proximity->setup(binding.proximityFinger, binding.side, FingerThumb, binding.side);

			std::shared_ptr<HOL::Gesture::BaseGesture::Gesture> triggerGesture = proximity;
			if (binding.modifier != GripModifier::None)
			{
				auto combo = HOL::Gesture::ComboGesture::Gesture::Create();
				combo->parameters.holdUntilAllReleased = false;
				combo->parameters.valueMode = HOL::Gesture::ComboGesture::ValueMode::Product;
				combo->addGesture(proximity);
				combo->addGesture(
					buildGripModifier(binding.modifier, binding.side, binding.proximityFinger));
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

			action->setTriggerGesture(triggerGesture);
		}
		else if (binding.kind == GestureKind::Chain)
		{
			action = HOL::ButtonAction::Create();
			action->setTriggerGesture(buildChainGesture(binding.side, binding.chainDirection));
		}
		else if (binding.kind == GestureKind::Grip)
		{
			auto gripGesture = buildGripGesture(binding.side);

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

			action->setTriggerGesture(gripGesture);
		}
		else if (binding.kind == GestureKind::SystemAim)
		{
			action = HOL::ButtonAction::Create();
			action->setTriggerGesture(buildSystemAimGesture(binding.side));
		}

		if (action)
		{
			addSinksForBinding(action, binding);
		}

		return action;
	}
} // namespace HOL::GestureBindings
