#include "HandOfLesserCore.h"
#include <thread>
#include "src/oculus/oculus_hacks.h"
#include "src/openxr/XrUtils.h"
#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include "src/vrchat/vrchat_osc.h"

using namespace HOL;
using namespace HOL::OpenXR;

void HandOfLesserCore::init(int serverPort)
{
	std::string runtimePath = HOL::OpenXR::getActiveOpenXRRuntimePath(1);
	std::string runtimeName = HOL::OpenXR::getActiveOpenXRRuntimeName(1);
	std::cout << "Active OpenXR Runtime is: " << runtimePath << std::endl;
	HOL::display::OpenXrRuntimeName = runtimeName;

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

	this->mUserInterface.init();
}

void HandOfLesserCore::start()
{
	this->mainLoop();
}

std::vector<const char*> HandOfLesserCore::getRequiredExtensions()
{
	return std::vector<const char*>();
}

void HandOfLesserCore::mainLoop()
{
	while (1)
	{
		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			doOpenXRStuff();
		}

		this->mUserInterface.onFrame();
		if (this->mUserInterface.shouldClose())
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::cout << "Exiting loop" << std::endl;
}

void HandOfLesserCore::doOpenXRStuff()
{
	XrTime time = this->mInstanceHolder.getTime();
	time += 1000000LL * (XrTime)HOL::settings::MotionPredictionMS;

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
	HOL::HandSide side = this->mVrchatOSC.swapTransmitSide();

	// Just generates it for now, still need to send.
	this->mVrchatOSC.generateOscOutput(this->mHandTracking.getHandPose(side), side);

	size_t size = this->mVrchatOSC.generateOscBundle(side);
	this->mTransport.send(9000, this->mVrchatOSC.getPacketBuffer(), size);

}

void HandOfLesserCore::sendUpdate()
{
	{
		HOL::HandTransformPacket packet = this->mHandTracking.getTransformPacket(HOL::LeftHand);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));

		packet = this->mHandTracking.getTransformPacket(HOL::RightHand);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::HandTransformPacket));
	}

	{
		HOL::ControllerInputPacket packet = this->mHandTracking.getInputPacket(HOL::LeftHand);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));

		packet = this->mHandTracking.getInputPacket(HOL::RightHand);
		this->mTransport.send(9006, (char*)&packet, sizeof(HOL::ControllerInputPacket));
	}
}
