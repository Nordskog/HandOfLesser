#include "settings.h"

namespace HOL::settings
{
	std::vector<GestureBinding> defaultGestureBindings()
	{
		std::vector<GestureBinding> bindings;
		auto addBinding = [&](const GestureBinding& binding) { bindings.push_back(binding); };

		// Joystick — Left hand, Middle finger proximity → Joystick target
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
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

		// Y (VRChat menu) — Left hand, Ring -> Middle -> Index
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
			b.kind = GestureKind::Chain;
			b.chainFingers = {
				HOL::FingerRing, HOL::FingerMiddle, HOL::FingerIndex, HOL::FingerLittle};
			b.chainLength = 3;
			b.target = InputTarget::Y;
			addBinding(b);
		}

		// X (VRChat mute) — Left hand, Index -> Middle -> Ring
		{
			GestureBinding b;
			b.side = HOL::LeftHand;
			b.kind = GestureKind::Chain;
			b.chainFingers = {
				HOL::FingerIndex, HOL::FingerMiddle, HOL::FingerRing, HOL::FingerLittle};
			b.chainLength = 3;
			b.target = InputTarget::X;
			addBinding(b);
		}

		// Toggle SteamVR Input — Right hand, Ring -> Middle -> Index
		{
			GestureBinding b;
			b.side = HOL::RightHand;
			b.kind = GestureKind::Chain;
			b.chainFingers = {
				HOL::FingerRing, HOL::FingerMiddle, HOL::FingerIndex, HOL::FingerLittle};
			b.chainLength = 3;
			b.target = InputTarget::Toggle_SteamVRInput;
			addBinding(b);
		}

		// System Aim — Both hands (Oculus only, but stored for convenience)
		for (int i = 0; i < HandSide::HandSide_MAX; i++)
		{
			GestureBinding b;
			b.side = (HOL::HandSide)i;
			b.kind = GestureKind::SystemAim;
			b.target = InputTarget::System;
			addBinding(b);
		}

		return bindings;
	}
} // namespace HOL::settings
