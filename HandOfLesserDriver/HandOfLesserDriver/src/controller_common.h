#pragma once

#include <HandOfLesserCommon.h>
#include "openvr_driver.h"

namespace HOL::ControllerCommon
{
	extern vr::DriverPose_t generatePose(HOL::HandTransformPacket* packet, bool deviceConnected);



}