#pragma once
#include <thread>
#include <HandOfLesserCommon.h>
#include "src/controller/emulated_controller_driver.h"
#include "src/controller/hooked_controller.h"

namespace HOL
{

	class HandOfLesser
	{
	public:
		void init();
		void cleanup();
		void runFrame();
		void addControllers();
		void addHookedController(uint32_t id,
								 HandSide side,
								 vr::IVRServerDriverHost* host,
								 vr::ITrackedDeviceServerDriver* driver);

		bool shouldPossess(uint32_t deviceId);
		bool shouldPossess(HookedController* controller);

		bool shouldEmulateControllers();
		bool hookedControllersFound();
		static HandOfLesser* Current; // Time to commit sins
		static HOL::settings::HandOfLesserSettings Config;

		EmulatedControllerDriver* getEmulatedController(HOL::HandSide side);
		HookedController* getHookedController(HOL::HandSide side);
		HookedController* getHookedControllerByDeviceId(uint32_t deviceId);
		GenericControllerInterface* GetActiveController(HOL::HandSide side);

	private:
		void ReceiveDataThread();

		bool mActive;

		std::thread my_pose_update_thread_;
		HOL::NativeTransport mTransport;

		std::unique_ptr<EmulatedControllerDriver> mEmulatedControllers[2];
		std::unique_ptr<HookedController> mHookedControllers[2];
	};
} // namespace HOL