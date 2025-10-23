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
		void addEmulatedControllers();
		void removeEmulatedControllers();
		HookedController* addHookedController(uint32_t id,
											  vr::IVRServerDriverHost* host,
											  vr::ITrackedDeviceServerDriver* driver,
											  vr::PropertyContainerHandle_t propertyContainer);

		void removeDuplicateDevices();

		bool shouldPossess(uint32_t deviceId);
		bool shouldPossess(HookedController* controller);

		bool shouldEmulateControllers();
		static HandOfLesser* Current; // Time to commit sins
		static HOL::settings::HandOfLesserSettings Config;

		EmulatedControllerDriver* getEmulatedController(HOL::HandSide side);
		bool isEmulatedController(vr::ITrackedDeviceServerDriver* driver);
		HookedController* getHookedController(HOL::HandSide side);
		HookedController* getHookedControllerByDeviceId(uint32_t deviceId);
		HookedController* getHookedControllerBySerial(std::string serial);
		HookedController*
		getHookedControllerByPropertyContainer(vr::PropertyContainerHandle_t container);
		HookedController* getHMD();
		HookedController*
		getHookedControllerByInputHandle(vr::VRInputComponentHandle_t inputHandle);
		GenericControllerInterface* GetActiveController(HOL::HandSide side);

		void requestEstimateControllerSide();

		float getControllerToHandDistance(HookedController* controller);
		bool isHandTrackingPrimary(HOL::HandSide side);
		bool shouldUseHandTracking(HookedController* controller);

		HOL::MultimodalPosePacket mLastMultimodalPosePacket;
		static HOL::state::TrackingState Tracking;
		static HOL::state::RuntimeState Runtime;

	private:
		void ReceiveDataThread();
		void estimateControllerSide();

		bool mActive;
		int mControllerSideEstimationAttemptCount = 0;

		bool mEstimateControllerSideWhenPositionValid = false;

		void handleConfigurationChange(HOL::settings::HandOfLesserSettings& newConfig);
		void updateControllerConnectionStates(bool forceUpdate = false);

		std::thread my_pose_update_thread_;
		HOL::NativeTransport mTransport;

		std::unique_ptr<EmulatedControllerDriver> mEmulatedControllers[2];
		std::vector<std::unique_ptr<HookedController>> mHookedControllers;
		HOL::HandTransformPacket mLastHandTransforms[HOL::HandSide_MAX]{};
		bool mHasHandTransform[HOL::HandSide_MAX]{false, false};
	};
} // namespace HOL
