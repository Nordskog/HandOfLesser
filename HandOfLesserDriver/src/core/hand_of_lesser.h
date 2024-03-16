#pragma once
#include <thread>
#include <HandOfLesserCommon.h>
#include "src/controller/emulated_controller_driver.h"
#include "src/controller/hooked_controller.h"

namespace HOL
{
	enum ControllerMode
	{
		NoControllerMode, EmulateControllerMode, HookedControllerMode, ControllerMode_MAX
	};

	class HandOfLesser
	{
	public:
		void init();
		void cleanup();
		void addControllers();
		void addHookedController(uint32_t id,
								 HandSide side,
								 vr::IVRServerDriverHost* host, vr::ITrackedDeviceServerDriver* driver);
		bool shouldPossess(uint32_t deviceId);
		bool shouldEmulateControllers();
		bool hookedControllersFound();
		static HandOfLesser* Current;	// Time to commit sins
	private:


		void ReceiveDataThread();

		EmulatedControllerDriver* getEmulatedController(HOL::HandSide side);
		HookedController* getHookedController(HOL::HandSide side);
		HookedController* getHookedControllerByDeviceId(uint32_t deviceId);
		GenericControllerInterface* GetActiveController(HOL::HandSide side);

		void runFrame();

		bool mActive;

		bool mLastPoseValid[2];

		ControllerMode mControllerMode;
		std::thread my_pose_update_thread_;
		HOL::NativeTransport mTransport;

		std::unique_ptr<EmulatedControllerDriver> mEmulatedControllers[2];
		std::unique_ptr<HookedController> mHookedControllers[2];
	};
}