#pragma once

#include <HandOfLesserCommon.h>
#include "emulated_controller_driver.h"

namespace HOL
{
	inline bool isLeftControllerRole(vr::ETrackedControllerRole role)
	{
		return role == vr::TrackedControllerRole_LeftHand;
	}

	inline const char* getEmulatedControllerModelName(EmulatedControllerProfile profile, bool left)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return left ? "HandOfLesser Touch Left" : "HandOfLesser Touch Right";
		}

		return left ? "Knuckles Left" : "Knuckles Right";
	}

	inline const char* getEmulatedControllerRenderModelName(EmulatedControllerProfile profile,
															 bool left)
	{
		// Keep index render models for now. Input profile controls the layout semantics.
		(void)profile;
		return left ? "{indexcontroller}valve_controller_knu_1_0_left"
					: "{indexcontroller}valve_controller_knu_1_0_right";
	}

	inline std::string getEmulatedControllerSerial(EmulatedControllerProfile profile,
												   const std::string& baseSerial)
	{
		return baseSerial
			   + (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch
					  ? "_touch"
					  : "_index");
	}

	inline const char* getEmulatedControllerResourceRoot(EmulatedControllerProfile profile)
	{
		return (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
				   ? "00handoflesser"
				   : "indexcontroller";
	}

	inline const char* getEmulatedControllerRegisteredType(EmulatedControllerProfile profile,
														   bool left)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return left ? "handoflesser/touch_left"
						: "handoflesser/touch_right";
		}

		return left ? "valve/index_controllerLHR-E217CD00"
					: "valve/index_controllerLHR-E217CD01";
	}

	inline const char* getEmulatedControllerInputProfilePath(EmulatedControllerProfile profile)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return "{00handoflesser}/input/touch_profile.json";
		}

		return "{indexcontroller}/input/index_controller_profile.json";
	}

	inline const char* getEmulatedControllerTypeString(EmulatedControllerProfile profile)
	{
		return (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
				   ? "handoflesser_touch"
				   : "knuckles";
	}

	// Maps index inputs to touch controller and vice-versa,
	// so we can emulate both without needing to do separate input paths
	inline std::string mapEmulatedInputPath(EmulatedControllerProfile profile,
											HandSide side,
											const std::string& inputPath)
	{
		auto inputType = INPUT_TYPES.find(inputPath);
		if (inputType == INPUT_TYPES.end())
		{
			return inputPath;
		}

		InputHandleType mappedType = inputType->second;

		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_Index)
		{
			switch (mappedType)
			{
				case InputHandleType::joystick_x:
					mappedType = InputHandleType::thumbstick_x;
					break;
				case InputHandleType::joystick_y:
					mappedType = InputHandleType::thumbstick_y;
					break;
				case InputHandleType::joystick_touch:
					mappedType = InputHandleType::thumbstick_touch;
					break;
				case InputHandleType::joystick_click:
					mappedType = InputHandleType::thumbstick_click;
					break;
				case InputHandleType::x_touch:
					mappedType = InputHandleType::a_touch;
					break;
				case InputHandleType::x_click:
					mappedType = InputHandleType::a_click;
					break;
				case InputHandleType::y_touch:
					mappedType = InputHandleType::b_touch;
					break;
				case InputHandleType::y_click:
					mappedType = InputHandleType::b_click;
					break;
				default:
					break;
			}

			return INPUT_PATHS[mappedType];
		}

		switch (mappedType)
		{
			case InputHandleType::thumbstick_x:
				mappedType = InputHandleType::joystick_x;
				break;
			case InputHandleType::thumbstick_y:
				mappedType = InputHandleType::joystick_y;
				break;
			case InputHandleType::thumbstick_touch:
				mappedType = InputHandleType::joystick_touch;
				break;
			case InputHandleType::thumbstick_click:
				mappedType = InputHandleType::joystick_click;
				break;
			default:
				break;
		}

		if (side == HandSide::LeftHand)
		{
			switch (mappedType)
			{
				case InputHandleType::a_touch:
					mappedType = InputHandleType::x_touch;
					break;
				case InputHandleType::a_click:
					mappedType = InputHandleType::x_click;
					break;
				case InputHandleType::b_touch:
					mappedType = InputHandleType::y_touch;
					break;
				case InputHandleType::b_click:
					mappedType = InputHandleType::y_click;
					break;
				default:
					break;
			}
		}
		else if (side == HandSide::RightHand)
		{
			switch (mappedType)
			{
				case InputHandleType::x_touch:
					mappedType = InputHandleType::a_touch;
					break;
				case InputHandleType::x_click:
					mappedType = InputHandleType::a_click;
					break;
				case InputHandleType::y_touch:
					mappedType = InputHandleType::b_touch;
					break;
				case InputHandleType::y_click:
					mappedType = InputHandleType::b_click;
					break;
				default:
					break;
			}
		}

		return INPUT_PATHS[mappedType];
	}
} // namespace HOL
