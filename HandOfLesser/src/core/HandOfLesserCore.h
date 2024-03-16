#pragma once

#include <memory>
#include "src/openxr/InstanceHolder.h"
#include "src/openxr/HandTracking.h"
#include "src/openxr/XrEventsInterface.h"
#include <HandOfLesserCommon.h>
#include "src/core/ui/user_interface.h"
#include "src/vrchat/vrchat_osc.h"

using namespace HOL;
using namespace HOL::OpenXR;
using namespace HOL::VRChat;

namespace HOL
{
	class HandOfLesserCore // : public XrEventsInterface
	{
	public:
		void init(int serverPort);
		void start();
		static HandOfLesserCore* Current; // Time to commit sinss

		virtual std::vector<const char*> getRequiredExtensions();

	private:
		InstanceHolder mInstanceHolder;
		HandTracking mHandTracking;
		UserInterface mUserInterface;
		VRChatOSC mVrchatOSC;
		NativeTransport mTransport;

		void mainLoop();
		void doOpenXRStuff();
		void doOscStuff();
		void sendUpdate();
	};
} // namespace HOL
