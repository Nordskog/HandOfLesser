//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include "openvr_driver.h"
#include <HandOfLesserCommon.h>

using namespace HOL::SteamVR;

class MyHandSimulation
{
public:
	void ComputeSkeletonTransforms(
		vr::ETrackedControllerRole role,
		const MyFingerCurls& curls,
		const MyFingerSplays& splays,
		vr::VRBoneTransform_t* out_transforms
	);
};
