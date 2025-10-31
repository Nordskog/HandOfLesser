#pragma once

#include <memory>
#include "src/openxr/InstanceHolder.h"
#include "src/openxr/HandTracking.h"
#include "src/openxr/XrEventsInterface.h"
#include <HandOfLesserCommon.h>
#include "src/core/ui/user_interface.h"
#include "src/vrchat/vrchat_osc.h"
#include <thread>
#include "src/vrchat/vrchat_input.h"
#include "src/steamvr/steamvr_input.h"
#include "src/steamvr/skeletal_input.h"
#include "src/openxr/body_tracking.h"
#include "src/core/features_manager.h"
#include "src/core/state_global.h"

using namespace HOL;
using namespace HOL::OpenXR;
using namespace HOL::VRChat;

namespace HOL
{
	class HandOfLesserCore // : public XrEventsInterface
	{
	public:
		HandOfLesserCore();
		void init(int serverPort);
		void start();
		static HandOfLesserCore* Current; // Time to commit sinss

		void syncSettings();
		void syncState();
		void saveSettings();
		void loadSettings();
		bool isDriverConnected() const;

		virtual std::vector<const char*> getRequiredExtensions();

		FeaturesManager featuresManager;

	private:
		InstanceHolder mInstanceHolder;
		HandTracking mHandTracking;
		BodyTracking mBodyTracking;
		UserInterface mUserInterface;
		VRChatOSC mVrchatOSC;
		VRChatInput mVrchatInput;
		SteamVR::SkeletalInput mSkeletalInput;
		SteamVR::SteamVRInput mSteamVRInput;
		NamedPipeTransport mDriverTransport;
		UdpTransport mOscTransport;

		std::thread mUserInterfaceThread;
		std::thread mReceiveThread;
		bool mActive = false;

		void userInterfaceLoop();
		void receiveDataThread();

		void mainLoop();
		void doOpenXRStuff();
		void sendOscData();
		void sendUpdate();
		void sendBodyTrackerData();
	};
} // namespace HOL
