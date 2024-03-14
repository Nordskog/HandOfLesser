#include "driverlog.h"
#include "hooks.h"


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
				// TODO: vive wands will probably swap sides after activating
				HandSide side = role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand
							   ? HandSide::LeftHand
							   : HandSide::RightHand;

				DriverLog("Got controller, side: %s", side == HandSide::LeftHand ? "Left" : "Right");
				HOL::HandOfLesser::Current->addHookedController(unWhichDevice, side, HOL::hooks::mLastDeviceDriverHost, _this);
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
	}

	namespace TrackedDevicePoseUpdated
	{
		Hook<TrackedDevicePoseUpdated::Signature>
			FunctionHook("IVRServerDriverHost006::TrackedDevicePoseUpdated");

		static void Detour(vr::IVRServerDriverHost* _this,
						   uint32_t unWhichDevice,
						   const vr::DriverPose_t& newPose,
						   uint32_t unPoseStructSize)
		{
			// Just do nothing if we are possessing controllers
			if (!HOL::HandOfLesser::Current->shouldPossess(unWhichDevice))
			{
				return TrackedDevicePoseUpdated::FunctionHook.originalFunc(
					_this, unWhichDevice, newPose, unPoseStructSize);
			}
			else
			{
				// Nothing!
			}
		};
	} // namespace TrackedDevicePoseUpdated

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

}