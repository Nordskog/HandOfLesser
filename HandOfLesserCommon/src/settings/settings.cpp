#include "settings.h"

namespace HOL::settings
{
	std::vector<GestureBinding> defaultGestureBindings()
	{
		std::vector<GestureBinding> bindings;
		auto addBinding = [&](const GestureBinding& binding) { bindings.push_back(binding); };

		// Joystick — Both hands, Middle finger proximity → Joystick target
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{
			GestureBinding b;
			b.side = (HOL::HandSide)i;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerMiddle;
			b.target = InputTarget::Joystick;
			addBinding(b);
		}

		// Trigger — Both hands, Index finger proximity with Closed grip modifier
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{
			GestureBinding b;
			b.side = (HOL::HandSide)i;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerIndex;
			b.modifiers = static_cast<uint32_t>(GestureModifier::ClosedHand);
			b.target = InputTarget::Trigger;
			addBinding(b);
		}

		// Grip — Both hands, 3-finger curl
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{
			GestureBinding b;
			b.side = (HOL::HandSide)i;
			b.kind = GestureKind::Grip;
			b.target = InputTarget::Grip;
			addBinding(b);
		}

		// Left System — Index tap and hold, auto release
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerIndex;
			b.modifiers = static_cast<uint32_t>(GestureModifier::Hold)
						  | static_cast<uint32_t>(GestureModifier::LookingAtHand);
			b.target = InputTarget::System;
			b.pressAndRelease = true;
			addBinding(b);
		}

		// Left X — Ring tap and hold
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerRing;
			b.modifiers = static_cast<uint32_t>(GestureModifier::Hold)
						  | static_cast<uint32_t>(GestureModifier::LookingAtHand);
			b.target = InputTarget::X;
			addBinding(b);
		}

		// Left Y — Pinky tap and hold
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerLittle;
			b.modifiers = static_cast<uint32_t>(GestureModifier::Hold)
						  | static_cast<uint32_t>(GestureModifier::LookingAtHand);
			b.target = InputTarget::Y;
			addBinding(b);
		}

		// Right X — Ring tap
		{
			GestureBinding b;
			b.side = HOL::RightHand;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerRing;
			b.target = InputTarget::X;
			addBinding(b);
		}

		// Right Y — Pinky tap and hold
		{
			GestureBinding b;
			b.side = HOL::RightHand;
			b.kind = GestureKind::Proximity;
			b.proximityFinger = HOL::FingerLittle;
			b.modifiers = static_cast<uint32_t>(GestureModifier::Hold)
						  | static_cast<uint32_t>(GestureModifier::LookingAtHand);
			b.target = InputTarget::Y;
			addBinding(b);
		}

		return bindings;
	}
} // namespace HOL::settings
