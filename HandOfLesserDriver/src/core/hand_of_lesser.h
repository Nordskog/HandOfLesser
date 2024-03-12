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
		void addHookedController(uint32_t id, HandSide side, vr::ITrackedDeviceServerDriver* driver);
	private:


		void ReceiveDataThread();

		void addControllers();



		EmulatedControllerDriver* getEmulatedController(HOL::HandSide side);
		HookedController* getHookedController(HOL::HandSide side);
		GenericControllerInterface* GetActiveController(HOL::HandSide side);

		void runFrame();

		bool mActive;

		ControllerMode mControllerMode;
		std::thread my_pose_update_thread_;
		HOL::NativeTransport mTransport;

		std::unique_ptr<EmulatedControllerDriver> mEmulatedControllers[2];
		std::unique_ptr<HookedController> mHookedControllers[2];
	};
}