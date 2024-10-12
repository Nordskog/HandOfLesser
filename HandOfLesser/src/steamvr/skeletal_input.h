#pragma once

#include <HandOfLesserCommon.h>
#include "src/openxr/openxr_hand.h"

#include <openvr_driver.h>

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

namespace HOL::SteamVR
{

	XrHandJointEXT ovrJointToOpenXR(HandSkeletonBone ovrBone);
	void getOpenXRJointLocation(XrHandJointLocationEXT* openXRJoints,
								HandSkeletonBone ovrBone,
								HOL::PoseLocation& out);

	class SkeletalInput
	{
	public:
		HOL::SkeletalPacket& getSkeletalPacket(OpenXRHand& hand, HandSide side);

	private:
		// Don't really need to separate them but eh
		HOL::SkeletalPacket mLeftPacket;
		HOL::SkeletalPacket mRightPacket;
	};
} // namespace HOL::SteamVR