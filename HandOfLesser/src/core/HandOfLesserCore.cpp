#include "HandOfLesserCore.h"
#include <thread>
#include "src/oculus/oculus_hacks.h"
#include "src/openxr/XrUtils.h"
#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include "src/vrchat/vrchat_osc.h"

using namespace HOL;
using namespace HOL::OpenXR;

HandOfLesserCore* HandOfLesserCore::Current = nullptr;

void HandOfLesserCore::init(int serverPort)
{
	this->Current = this;

	// The offsets and stuff we should should depend on other settings,
	// but for the time being taking control over existing touch controllers
	// will be the goal, so default to that for now.
	HOL::settings::restoreDefaultControllerOffset(ControllerType::OculusTouch_Airlink);

	std::string runtimePath = HOL::OpenXR::getActiveOpenXRRuntimePath(1);
	std::string runtimeName = HOL::OpenXR::getActiveOpenXRRuntimeName(1);
	std::cout << "Active OpenXR Runtime is: " << runtimePath << std::endl;
	HOL::display::OpenXrRuntimeName = runtimeName;

	if (runtimeName.find("virtualdesktop") != std::string::npos)
	{
		HOL::display::IsVDXR = true;
		std::cout << "Runtime is VDXR, will expect being fed lies" << std::endl;
	}

	this->mInstanceHolder.init();

	if (this->mInstanceHolder.getState() != OpenXrState::Failed)
	{
		this->mInstanceHolder.beginSession();

		// Run everything else even if we don't have valid OpenXR so we
		// can test UI and stuff
		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			this->mHandTracking.init(this->mInstanceHolder.mInstance,
									 this->mInstanceHolder.mSession);

			// Airlink doesn't support headless and requires hax
			// As of writing, the only other supported runtime is VDXR
			if (!this->mInstanceHolder.isHeadless())
			{
				HOL::hacks::fixOvrSessionStateRestriction();
			}
		}
	}
	this->mTransport.init(serverPort);
}

void HandOfLesserCore::start()
{
	this->mUserInterfaceThread = std::thread(&HandOfLesserCore::userInterfaceLoop, this);
	this->mainLoop();
}

std::vector<const char*> HandOfLesserCore::getRequiredExtensions()
{
	return std::vector<const char*>();
}

void HOL::HandOfLesserCore::userInterfaceLoop()
{
	this->mUserInterface.init();

	while (1)
	{
		// This vsyncs so no need to sleep manually
		this->mUserInterface.onFrame();
		if (this->mUserInterface.shouldTerminate())
		{
			break;
		}
	}

	this->mUserInterface.terminate();
}

void HandOfLesserCore::mainLoop()
{
	while (1)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		if (this->mUserInterface.shouldTerminate())
		{
			break;
		}

		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			doOpenXRStuff();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(Config.general.UpdateIntervalMS));
	}

	std::cout << "Exiting loop" << std::endl;
	this->mUserInterfaceThread.join();
}

void HandOfLesserCore::doOpenXRStuff()
{
	XrTime time = this->mInstanceHolder.getTime();
	time += 1000000LL * (XrTime)Config.general.MotionPredictionMS;

	this->mInstanceHolder.pollEvent();

	this->mHandTracking.updateHands(this->mInstanceHolder.mStageSpace, time);
	this->mHandTracking.updateInputs();

	this->sendUpdate();

	// OSC is less time critically and should probably happen after we send the controller packet
	doOscStuff();

	return;
}

void HandOfLesserCore::doOscStuff()
{
	// This will generate everything needed for all transmit types
	this->mVrchatOSC.generateOscOutput(this->mHandTracking.getHandPose(HandSide::LeftHand),
									   this->mHandTracking.getHandPose(HandSide::RightHand));

	size_t size = 0;

	// Always send full, expect when testing remote stuff locally because it will break things
	if (Config.vrchat.sendFull)
	{
		size = this->mVrchatOSC.generateOscBundleFull();
		this->mTransport.send(9000, this->mVrchatOSC.getPacketBuffer(), size);
	}

	if (Config.vrchat.sendAlternating)
	{
		size = this->mVrchatOSC.generateOscBundleAlternating();
		this->mTransport.send(9000, this->mVrchatOSC.getPacketBuffer(), size);
	}

	if (Config.vrchat.sendPacked)
	{
		size = this->mVrchatOSC.generateOscBundlePacked();
		this->mTransport.send(9000, this->mVrchatOSC.getPacketBuffer(), size);
	}
}

void HandOfLesserCore::sendUpdate()
{
	for (int i = 0; i < HandSide_MAX; i++)
	{
		HandPose hand = this->mHandTracking.getHandPose((HandSide)i);

		// No point in sending any new data if the data is the same as last time.
		if (!hand.poseStale)
		{
			HOL::HandTransformPacket transPacket
				= this->mHandTracking.getTransformPacket((HandSide)i);
			this->mTransport.send(9006, (char*)&transPacket, sizeof(HOL::HandTransformPacket));

			HOL::ControllerInputPacket inputPacket
				= this->mHandTracking.getInputPacket((HandSide)i);
			this->mTransport.send(9006, (char*)&inputPacket, sizeof(HOL::ControllerInputPacket));
		}
	}
}

void HOL::HandOfLesserCore::syncSettings()
{
	HOL::SettingsPacket packet;
	packet.config = HOL::Config;
	this->mTransport.send(9006, (char*)&packet, sizeof(HOL::SettingsPacket));
}
