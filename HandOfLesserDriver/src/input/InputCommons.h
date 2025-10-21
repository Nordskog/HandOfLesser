#pragma once

#include <string>
#include <openvr_driver.h>

namespace HOL
{
	enum ControllerInputType
	{
		Boolean,
		Scalar,
		Skeleton,
		ControllerInputType_MAX
	};

	struct ControllerInputHandle
	{
		std::string inputPath = "";
		ControllerInputType type = ControllerInputType::Boolean;
		vr::VRInputComponentHandle_t handle = 0;
	};

} // namespace HOL
