#pragma once

#include <openvr_driver.h>
#include "src/core/hand_of_lesser.h"
#include "Hooking.h"

namespace HOL::hooks
{
	namespace TrackedDeviceAdded006
	{
		using Signature = void (*)(vr::IVRServerDriverHost*,
											const char* pchDeviceSerialNumber,
											vr::ETrackedDeviceClass eDeviceClass,
											vr::ITrackedDeviceServerDriver* pDriver);

		extern Hook<TrackedDeviceAdded006::Signature> FunctionHook;
	}

	namespace GetGenericInterface
	{
		using Signature = void* (*)(vr::IVRDriverContext*, const char*, vr::EVRInitError*);

		extern Hook<Signature> FunctionHook;
	}

	namespace TrackedDeviceActivate
	{
		using Signature = vr::EVRInitError (*)(vr::ITrackedDeviceServerDriver*, uint32_t);

		extern Hook<Signature> FunctionHook;

	}

	extern HOL::HandOfLesser* HandOfLesserInstance;
	extern void InjectHooks(vr::IVRDriverContext* pDriverContext, HOL::HandOfLesser* hol);
	extern void DisableHooks();
}