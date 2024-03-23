#pragma once

#include <HandOfLesserCommon.h>
#include "openvr_driver.h"
#include <random>

namespace HOL::ControllerCommon
{
	extern vr::DriverPose_t generatePose(HOL::HandTransformPacket* packet, bool deviceConnected);

	extern vr::DriverPose_t addJitter(const vr::DriverPose_t& existingPose);

}