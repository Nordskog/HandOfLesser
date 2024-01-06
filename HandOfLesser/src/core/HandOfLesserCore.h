#pragma once

#include <memory>
#include "src/openxr/InstanceHolder.h"
#include "src/openxr/HandTracking.h"
#include "src/openxr/XrEventsInterface.h"
#include <HandOfLesserCommon.h>
#include "src/core/ui/user_interface.h"
#include "src/vrchat/vrchat_osc.h"

class HandOfLesserCore // : public XrEventsInterface
{
public:
	void init(int serverPort);
	void start();

	virtual std::vector<const char*> getRequiredExtensions();

private:
	std::unique_ptr<InstanceHolder> mInstanceHolder;
	std::unique_ptr<HandTracking> mHandTracking;
	std::unique_ptr<UserInterface> mUserInterface;
	HOL::VRChat::VRChatOSC mVrchatOSC;
	HOL::NativeTransport mTransport;

	void mainLoop();
	void doOpenXRStuff();
	void doOscStuff();
	void sendUpdate();
};