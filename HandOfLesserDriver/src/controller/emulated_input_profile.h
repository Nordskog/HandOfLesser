#pragma once

#include <HandOfLesserCommon.h>
#include <cstring>
#include <string>
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
			return left ? "Oculus Quest2 (Left Controller)"
						: "Oculus Quest2 (Right Controller)";
		}

		return left ? "Knuckles Left" : "Knuckles Right";
	}

	inline const char* getEmulatedControllerRenderModelName(EmulatedControllerProfile profile,
															 bool left)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return left ? "oculus_quest2_controller_left"
						: "oculus_quest2_controller_right";
		}

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
				   ? "oculus"
				   : "indexcontroller";
	}

	inline const char* getEmulatedControllerRegisteredType(EmulatedControllerProfile profile,
														   bool left)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return left ? "oculus/WMHD315M3010GV_Controller_Left"
						: "oculus/WMHD315M3010GV_Controller_Right";
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
				   ? "oculus_touch"
				   : "knuckles";
	}

	inline std::string getEmulatedControllerIconPath(EmulatedControllerProfile profile,
													 bool left,
													 const char* state)
	{
		if (profile == EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			std::string extension = ".png";
			if (strcmp(state, "searching") == 0 || strcmp(state, "searching_alert") == 0)
			{
				extension = ".gif";
			}

			return std::string("{oculus}/icons/rifts_")
				   + (left ? "left" : "right")
				   + "_controller_"
				   + state
				   + extension;
		}

		std::string extension = ".png";
		if (strcmp(state, "searching") == 0 || strcmp(state, "searching_alert") == 0)
		{
			extension = ".gif";
		}

		return std::string("{00handoflesser}/icons/controller_status_") + state + extension;
	}

	inline void setEmulatedControllerIconProperties(vr::CVRPropertyHelpers* props,
													vr::PropertyContainerHandle_t container,
													EmulatedControllerProfile profile,
													bool left)
	{
		if (profile != EmulatedControllerProfile::EmulatedControllerProfile_OculusTouch)
		{
			return;
		}

		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceOff_String,
			getEmulatedControllerIconPath(profile, left, "off").c_str());
		props->SetStringProperty( 
			container,
			vr::Prop_NamedIconPathDeviceSearching_String,
			getEmulatedControllerIconPath(profile, left, "searching").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceSearchingAlert_String,
			getEmulatedControllerIconPath(profile, left, "searching_alert").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceReady_String,
			getEmulatedControllerIconPath(profile, left, "ready").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceReadyAlert_String,
			getEmulatedControllerIconPath(profile, left, "ready_alert").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceNotReady_String,
			getEmulatedControllerIconPath(profile, left, "error").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceStandby_String,
			getEmulatedControllerIconPath(profile, left, "standby").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathDeviceAlertLow_String,
			getEmulatedControllerIconPath(profile, left, "ready_low").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathControllerLeftDeviceOff_String,
			getEmulatedControllerIconPath(profile, true, "off").c_str());
		props->SetStringProperty(
			container,
			vr::Prop_NamedIconPathControllerRightDeviceOff_String,
			getEmulatedControllerIconPath(profile, false, "off").c_str());
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
