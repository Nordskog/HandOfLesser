#include "driverlog.h"
#include "hooks.h"


namespace HOL::hooks
{
	HOL::HandOfLesser* HandOfLesserInstance;

	namespace TrackedDeviceActivate
	{
		Hook<TrackedDeviceActivate::Signature> FunctionHook("ITrackedDeviceServerDriver::Activate");

		static vr::EVRInitError Detour(vr::ITrackedDeviceServerDriver* _this,
									   uint32_t unWhichDevice)
		{
			DriverLog("TrackedDeviceActivate!");
			DriverLog("Device ID: %lld", (long long)unWhichDevice);

			// Let original function run before we do anything
			auto ret = TrackedDeviceActivate::FunctionHook.originalFunc(_this, unWhichDevice);

			// Get the role
			auto props = vr::VRProperties();
			vr::PropertyContainerHandle_t container
				= props->TrackedDeviceToPropertyContainer(unWhichDevice);

			vr::ETrackedControllerRole role
				= (vr::ETrackedControllerRole)props->GetInt32Property(container, vr::Prop_ControllerRoleHint_Int32);

			// Take note of any controllers
			if (role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand
				|| role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand)
			{
				HandSide side = role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand
							   ? HandSide::LeftHand
							   : HandSide::RightHand;

				DriverLog("Got controller, side: %s", side == HandSide::LeftHand ? "Left" : "Right");
				HandOfLesserInstance->addHookedController(unWhichDevice, side, _this);

			}

			return ret;
		}
	} // namespace TrackedDeviceActivate

	namespace TrackedDeviceAdded006
	{
		Hook<TrackedDeviceAdded006::Signature> FunctionHook("IVRServerDriverHost006::TrackedDeviceAdded");

		static void Detour(	vr::IVRServerDriverHost* _this,
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
			}
			else
			{
				DriverLog("Some other device added, class: %d", eDeviceClass);
			}

			return TrackedDeviceAdded006::FunctionHook.originalFunc(
				_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
		};
	}

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
			}

			return originalInterface;
		};
	} // namespace GetGenericInterface

	void InjectHooks(vr::IVRDriverContext* pDriverContext, HOL::HandOfLesser* hol)
	{
		HOL::hooks::HandOfLesserInstance = hol;
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

}