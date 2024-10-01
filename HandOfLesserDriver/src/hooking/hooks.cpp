#include "driverlog.h"
#include "hooks.h"
#include "src/controller/controller_common.h"

namespace HOL::hooks
{
	namespace TrackedDeviceActivate
	{
		Hook<TrackedDeviceActivate::Signature> FunctionHook("ITrackedDeviceServerDriver::Activate");

		static vr::EVRInitError Detour(vr::ITrackedDeviceServerDriver* _this,
									   uint32_t unWhichDevice)
		{
			DriverLog("TrackedDeviceActivate!");
			DriverLog("Device ID: %lld", (long long)unWhichDevice);

			// Get the role
			auto props = vr::VRProperties();
			vr::PropertyContainerHandle_t container
				= props->TrackedDeviceToPropertyContainer(unWhichDevice);

			// Add controller, get a pointer so we can set side later.
			HookedController* controller = HOL::HandOfLesser::Current->addHookedController(
				unWhichDevice, HOL::hooks::mLastDeviceDriverHost, _this, container);
			
			// Let original function run. This will add all the inputs, which is why
			// we need to add the controller first.
			auto ret = TrackedDeviceActivate::FunctionHook.originalFunc(_this, unWhichDevice);

			// These devices should not be populated until activate has been called,
			// but for some devices they are. Weird.
			std::string serial = props->GetStringProperty(container, vr::Prop_SerialNumber_String);
			vr::ETrackedDeviceClass deviceClass = (vr::ETrackedDeviceClass)props->GetInt32Property(
				container, vr::Prop_DeviceClass_Int32);
			vr::ETrackedControllerRole role = (vr::ETrackedControllerRole)props->GetInt32Property(
				container, vr::Prop_ControllerRoleHint_Int32);

			DriverLog("Device serial: %s", serial.c_str());
			DriverLog("Device class: %d", deviceClass);
			DriverLog("Device role: %d", role);

			controller->lateInit(serial, deviceClass, role);

			if (deviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
			{
				// TODO: vive wands will probably swap sides after activating
				HandSide side = HandSide::HandSide_MAX;
				switch (role)
				{
					case vr::ETrackedControllerRole::TrackedControllerRole_LeftHand:
						side = HandSide::LeftHand;
						break;
					case vr::ETrackedControllerRole::TrackedControllerRole_RightHand:
						side = HandSide::RightHand;
						break;
					default:
						side = HandSide::HandSide_MAX;
				}

				if (side == HandSide_MAX)
				{
					// Not a controller. I think ViveWands are assigned a side,
					// it just may swap them dynamically.
					// We should probably remove these.
					DriverLog("Got controller but no side assigned");
				}
				else
				{
					DriverLog("Got controller, side: %s",
							  side == HandSide::LeftHand ? "Left" : "Right");
					controller->setSide(side);
				}
			}
			else
			{
				DriverLog("Some other device activated, class: %d", deviceClass);
			}

			HandOfLesser::Current->removeDuplicateDevices();


			return ret;
		}
	} // namespace TrackedDeviceActivate

	namespace PollNextEvent
	{
		Hook<PollNextEvent::Signature> FunctionHook("ITrackedDeviceServerDriver::PollNextEvent");

		static bool
		Detour(vr::IVRServerDriverHost* _this, vr::VREvent_t* pEvent, uint32_t uncbVREvent)
		{
			//DriverLog("PollNextEvent!");
			auto ret = PollNextEvent::FunctionHook.originalFunc(_this, pEvent, uncbVREvent);

			if (ret)
			{
				if (pEvent->eventType == vr::EVREventType::VREvent_TrackedDeviceRoleChanged)
				{
					DriverLog("Received TrackedDeviceRoleChanged event, updating unhanded controllers");
					HandOfLesser::Current->requestEstimateControllerSide();
				}
			}

			return ret;
		}
	} // namespace PollNextEvent

	namespace TrackedDeviceAdded006
	{
		Hook<TrackedDeviceAdded006::Signature>
			FunctionHook("IVRServerDriverHost006::TrackedDeviceAdded");

		static bool Detour(vr::IVRServerDriverHost* _this,
						   const char* pchDeviceSerialNumber,
						   vr::ETrackedDeviceClass eDeviceClass,
						   vr::ITrackedDeviceServerDriver* pDriver)
		{
			DriverLog("TrackedDeviceAdded006!");

			if (!IHook::Exists(TrackedDeviceActivate::FunctionHook.name))
			{
				DriverLog("Adding activate hook");

				TrackedDeviceActivate::FunctionHook.CreateHookInObjectVTable(
					pDriver, 0, &TrackedDeviceActivate::Detour);
				IHook::Register(&TrackedDeviceActivate::FunctionHook);
			}

			if (eDeviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
			{
				DriverLog("Controller added!");

				// TODO: We only have access to the driverHost here, but don't know the ID
				// or the role of the controller yet. Activate() should be called
				// immediately afterwards, so the only concern is when devices are
				// deactivated and re-activated again. Think about that later.
				HOL::hooks::mLastDeviceDriverHost = _this;
			}
			else
			{
				DriverLog("Some other device added, class: %d", eDeviceClass);
			}

			return TrackedDeviceAdded006::FunctionHook.originalFunc(
				_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
		};
	} // namespace TrackedDeviceAdded006

	namespace TrackedDevicePoseUpdated
	{
		Hook<TrackedDevicePoseUpdated::Signature>
			FunctionHook("IVRServerDriverHost006::TrackedDevicePoseUpdated");

		static void Detour(vr::IVRServerDriverHost* _this,
						   uint32_t unWhichDevice,
						   const vr::DriverPose_t& newPose,
						   uint32_t unPoseStructSize)
		{
			auto& config = HOL::HandOfLesser::Current->Config;

			HookedController* controller
				= HOL::HandOfLesser::Current->getHookedControllerByDeviceId(unWhichDevice);

			if (controller != nullptr)
			{
				controller->setLastOriginalPoseState(newPose.deviceIsConnected
													 && newPose.poseIsValid);

				if (newPose.poseIsValid && newPose.deviceIsConnected)
				{
					controller->lastOriginalPose = newPose;
				}

				// reset frame counter
				controller->framesSinceLastPoseUpdate = 0;
				
				auto controllerMode = config.handPose.mControllerMode;
				if (controllerMode == ControllerMode::HookedControllerMode
					|| controllerMode == ControllerMode::OffsetControllerMode)
				{
					if (controllerMode == ControllerMode::HookedControllerMode)
					{
						// Just do nothing if we are possessing controllers
						if (HOL::HandOfLesser::Current->shouldPossess(controller))
						{
							// Prevent original function from being called
							return;
						}
					}
					else if (controllerMode == ControllerMode::OffsetControllerMode
							 && controller->canPossess())
					{
						// Modify pose by offset, if we are hand-tracking.
						// Casting to non-cast maybe bad, but don't want to create new one
						// constantly.
						HOL::ControllerCommon::offsetPose((vr::DriverPose_t&)newPose,
														  controller->getSide(),
														  config.handPose.PositionOffset,
														  config.handPose.OrientationOffset);
					}
				}
			}


			// Make sure to return early if we don't want to call the original function
			return TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				_this, unWhichDevice, newPose, unPoseStructSize);
		};
	} // namespace TrackedDevicePoseUpdated

	namespace CreateBooleanComponent
	{
		Hook<CreateBooleanComponent::Signature>
			FunctionHook("IVRDriverInput003::CreateBooleanComponent");

		static vr::EVRInputError Detour(vr::IVRDriverInput* _this,
										vr::PropertyContainerHandle_t ulContainer,
										const char* pchName,
										vr::VRInputComponentHandle_t* pHandle)
		{
			// pHandle is populated by this function, so let it run first.
			auto ret = CreateBooleanComponent::FunctionHook.originalFunc(
				_this, ulContainer, pchName, pHandle);

			// Get the controller by serial
			auto props = vr::VRProperties();
			std::string serial
				= props->GetStringProperty(ulContainer, vr::Prop_SerialNumber_String);

			// We hook this to get a reference to the IVRDriverInput for each controller.
			// We can identify the controller by the PropertyContainer, because it is unique
			// to each controller.
			HOL::HookedController* controller
				= HOL::HandOfLesser::Current->getHookedControllerByPropertyContainer(ulContainer);
			if (controller != nullptr)
			{
				controller->driverInput = _this;

				// Also store the path and handle of each input
				ControllerInputHandle input = {
					.inputPath = pchName, .type = ControllerInputType::Boolean, .handle = *pHandle};
				uint64_t key = *pHandle; // Something is fucked
				controller->inputHandles[key] = input;
				controller->inputHandlesByName[pchName] = *pHandle;
			}

			return ret;
		};
	} // namespace CreateBooleanComponent

	namespace CreateScalarComponent
	{
		Hook<CreateScalarComponent::Signature>
			FunctionHook("IVRDriverInput003::CreateScalarComponent");

		static vr::EVRInputError Detour(vr::IVRDriverInput* _this,
										vr::PropertyContainerHandle_t ulContainer,
										const char* pchName,
										vr::VRInputComponentHandle_t* pHandle,
										vr::EVRScalarType eType,
										vr::EVRScalarUnits eUnits)
		{
			// pHandle is populated by this function, so let it run first.
			auto ret = CreateScalarComponent::FunctionHook.originalFunc(
				_this, ulContainer, pchName, pHandle, eType, eUnits);

			// Get the controller by serial
			auto props = vr::VRProperties();
			std::string serial
				= props->GetStringProperty(ulContainer, vr::Prop_SerialNumber_String);

			// We hook this to get a reference to the IVRDriverInput for each controller.
			// We can identify the controller by the PropertyContainer, because it is unique
			// to each controller.
			HOL::HookedController* controller
				= HOL::HandOfLesser::Current->getHookedControllerByPropertyContainer(ulContainer);
			if (controller != nullptr)
			{
				controller->driverInput = _this;

				// Also store the path and handle of each input
				ControllerInputHandle input = {
					.inputPath = pchName, .type = ControllerInputType::Scalar, .handle = *pHandle};
				uint64_t key = *pHandle; // Something is fucked
				controller->inputHandles[key] = input;
				controller->inputHandlesByName[pchName] = *pHandle;
			}

			return ret;
		};
	} // namespace CreateScalarComponent

	namespace UpdateBooleanComponent
	{
		Hook<UpdateBooleanComponent::Signature>
			FunctionHook("IVRDriverInput003::UpdateBooleanComponent");

		static vr::EVRInputError Detour(vr::IVRDriverInput* _this,
										vr::VRInputComponentHandle_t ulComponent,
										bool bNewValue,
										double fTimeOffset)
		{
			auto& config = HOL::HandOfLesser::Current->Config;
			HookedController* controller
				= HandOfLesser::Current->getHookedControllerByInputHandle(ulComponent);

			//TODO we shouldn't do anything here if we're not... doing stuff.
			// Kinda wasteful.

			if (controller != nullptr)
			{
				auto& inputHandle = controller->inputHandles[ulComponent];

				// For the time being allow system gesture
				if (inputHandle.inputPath.find("system") == std::string::npos)
				{
									/*
				DriverLog("Controller: %s, Button: %s, value: %s",
							controller->serial.c_str(),
							inputHandle.inputPath.c_str(),
							bNewValue ? "True" : "False");
							*/
					auto controllerMode = config.handPose.mControllerMode;
					if (controller->canPossess())
					{
						if (controllerMode == ControllerMode::HookedControllerMode
							|| controllerMode == ControllerMode::OffsetControllerMode)
						{
							if (config.input.blockControllerInputWhileHandTracking)
							{
								// DriverLog("Blocked!");
								return vr::EVRInputError::VRInputError_None;
							}
						}
					}
				}	
			}
			else
			{
				DriverLog("Button on unknown controller!");
			}

			// Make sure to return early if we don't want to call the original function
			return UpdateBooleanComponent::FunctionHook.originalFunc(
				_this, ulComponent, bNewValue, fTimeOffset);
		};
	} // namespace UpdateBooleanComponent

	namespace UpdateScalarComponent
	{
		Hook<UpdateScalarComponent::Signature>
			FunctionHook("IVRDriverInput003::UpdateScalarComponent");

		static vr::EVRInputError Detour(vr::IVRDriverInput* _this,
										vr::VRInputComponentHandle_t ulComponent,
										float fNewValue,
										double fTimeOffset)
		{
			auto& config = HOL::HandOfLesser::Current->Config;
			HookedController* controller
				= HandOfLesser::Current->getHookedControllerByInputHandle(ulComponent);

			// TODO we shouldn't do anything here if we're not... doing stuff.
			//  Kinda wasteful.

			if (controller != nullptr)
			{
				/*
				auto& inputHandle = controller->inputHandles[ulComponent];

				DriverLog("Controller: %s, Button: %s, value: %.3f",
							controller->serial.c_str(),
							inputHandle.inputPath.c_str(),
							bNewValue);
							*/
				auto controllerMode = config.handPose.mControllerMode;
				if (controller->canPossess())
				{
					if (controllerMode == ControllerMode::HookedControllerMode
						|| controllerMode == ControllerMode::OffsetControllerMode)
					{
						if (config.input.blockControllerInputWhileHandTracking)
						{
							//DriverLog("Blocked!");
							return vr::EVRInputError::VRInputError_None;
						}
					}
				}
				
			}
			else
			{
				DriverLog("Scalar on unknown controller!");
			}

			// Make sure to return early if we don't want to call the original function
			return UpdateScalarComponent::FunctionHook.originalFunc(
				_this, ulComponent, fNewValue, fTimeOffset);
		};
	} // namespace UpdateScalarComponent

	namespace GetGenericInterface
	{
		Hook<GetGenericInterface::Signature> FunctionHook("IVRDriverContext::GetGenericInterface");

		static void* Detour(vr::IVRDriverContext* _this,
							const char* pchInterfaceVersion,
							vr::EVRInitError* peError)
		{
			void* originalInterface = GetGenericInterface::FunctionHook.originalFunc(
				_this, pchInterfaceVersion, peError);

			std::string iface(pchInterfaceVersion);
			if (iface == "IVRServerDriverHost_006")
			{
				DriverLog("Interface version: %s", iface.c_str());

				if (!IHook::Exists(TrackedDeviceAdded006::FunctionHook.name))
				{
					DriverLog("Adding added hook");

					TrackedDeviceAdded006::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 0, &TrackedDeviceAdded006::Detour);
					IHook::Register(&TrackedDeviceAdded006::FunctionHook);
				}

				if (!IHook::Exists(TrackedDevicePoseUpdated::FunctionHook.name))
				{
					DriverLog("Adding poseupdated hook");

					TrackedDevicePoseUpdated::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 1, &TrackedDevicePoseUpdated::Detour);
					IHook::Register(&TrackedDevicePoseUpdated::FunctionHook);
				}

				if (!IHook::Exists(PollNextEvent::FunctionHook.name))
				{
					DriverLog("Adding PollNextEvent hook");

					PollNextEvent::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 5, &PollNextEvent::Detour);
					IHook::Register(&PollNextEvent::FunctionHook);
				}
			}

			if (iface.find("IVRDriverInput") != std::string::npos)
			{
				DriverLog("Found Input interface: %s", iface.c_str());

				if (!IHook::Exists(UpdateBooleanComponent::FunctionHook.name))
				{
					DriverLog("Adding UpdateBooleanComponent hook");

					UpdateBooleanComponent::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 1, &UpdateBooleanComponent::Detour);
					IHook::Register(&UpdateBooleanComponent::FunctionHook);
				}

				if (!IHook::Exists(UpdateScalarComponent::FunctionHook.name))
				{
					DriverLog("Adding UpdateScalarComponent hook");

					UpdateScalarComponent::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 3, &UpdateScalarComponent::Detour);
					IHook::Register(&UpdateScalarComponent::FunctionHook);
				}

				if (!IHook::Exists(CreateBooleanComponent::FunctionHook.name))
				{
					DriverLog("Adding CreateBooleanComponent hook");

					CreateBooleanComponent::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 0, &CreateBooleanComponent::Detour);
					IHook::Register(&CreateBooleanComponent::FunctionHook);
				}

				if (!IHook::Exists(CreateScalarComponent::FunctionHook.name))
				{
					DriverLog("Adding CreateScalarComponent hook");

					CreateScalarComponent::FunctionHook.CreateHookInObjectVTable(
						originalInterface, 2, &CreateScalarComponent::Detour);
					IHook::Register(&CreateScalarComponent::FunctionHook);
				}
			}

			return originalInterface;
		};
	} // namespace GetGenericInterface

	vr::IVRServerDriverHost* mLastDeviceDriverHost = nullptr;

	void InjectHooks(vr::IVRDriverContext* pDriverContext)
	{
		auto err = MH_Initialize();
		if (err == MH_OK)
		{
			GetGenericInterface::FunctionHook.CreateHookInObjectVTable(
				pDriverContext, 0, &GetGenericInterface::Detour);
			IHook::Register(&GetGenericInterface::FunctionHook);
		}
		else
		{
			DriverLog("MH_Initialize error: %s", MH_StatusToString(err));
		}
	}

	void DisableHooks()
	{
		IHook::DestroyAll();
		MH_Uninitialize();
	}

} // namespace HOL::hooks