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
	} // namespace TrackedDeviceAdded006

	namespace GetGenericInterface
	{
		using Signature = void* (*)(vr::IVRDriverContext*, const char*, vr::EVRInitError*);

		extern Hook<Signature> FunctionHook;
	} // namespace GetGenericInterface

	namespace TrackedDeviceActivate
	{
		using Signature = vr::EVRInitError (*)(vr::ITrackedDeviceServerDriver*, uint32_t);

		extern Hook<Signature> FunctionHook;
	} // namespace TrackedDeviceActivate

	namespace UpdateBooleanComponent
	{
		using Signature = vr::EVRInputError (*)(vr::IVRDriverInput*,
												vr::VRInputComponentHandle_t ulComponent,
												bool bNewValue,
												double fTimeOffset);

		extern Hook<Signature> FunctionHook;
	} // namespace UpdateBooleanComponent

	namespace CreateBooleanComponent
	{
		using Signature = vr::EVRInputError (*)(vr::IVRDriverInput*,
												vr::PropertyContainerHandle_t ulContainer,
												const char* pchName,
												vr::VRInputComponentHandle_t* pHandle);

		extern Hook<Signature> FunctionHook;
	} // namespace CreateBooleanComponent

	namespace CreateScalarComponent
	{
		using Signature = vr::EVRInputError (*)(vr::IVRDriverInput*,
												vr::PropertyContainerHandle_t  ulContainer, 
												const char *pchName, 
												vr::VRInputComponentHandle_t *pHandle,
												vr::EVRScalarType eType, 
												vr::EVRScalarUnits eUnits);

		extern Hook<Signature> FunctionHook;
	} // namespace CreateScalarComponent

	namespace UpdateScalarComponent
	{
		using Signature = vr::EVRInputError (*)(vr::IVRDriverInput*,
												vr::VRInputComponentHandle_t ulComponent,
												float fNewValue,
												double fTimeOffset);

		extern Hook<Signature> FunctionHook;
	} // namespace UpdateScalarComponent

	namespace TrackedDevicePoseUpdated
	{
		using Signature
			= void (*)(vr::IVRServerDriverHost*, uint32_t, const vr::DriverPose_t&, uint32_t);

		extern Hook<TrackedDevicePoseUpdated::Signature> FunctionHook;
	} // namespace TrackedDevicePoseUpdated

	namespace PollNextEvent
	{ 
		using Signature = bool (*)(vr::IVRServerDriverHost*, vr::VREvent_t*, uint32_t);

		extern Hook<PollNextEvent::Signature> FunctionHook;
	} // namespace PollNextEvent

	// Set in TrackedDeviceAdded006(), should be directly before the corresponding device is
	// activated. If you get the wrong driverHost we won't be able to update the pose.
	extern vr::IVRServerDriverHost* mLastDeviceDriverHost;
	extern void InjectHooks(vr::IVRDriverContext* pDriverContext);
	extern void DisableHooks();
} // namespace HOL::hooks