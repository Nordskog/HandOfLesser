#include "HandOfLesserCore.h"
#include <thread>
#include "src/oculus/oculus_hacks.h"
#include "src/openxr/XrUtils.h"
#include "src/core/settings_global.h"
#include "src/core/state_global.h"
#include "src/vrchat/vrchat_osc.h"
#include <fstream>
#include <cstring>

using namespace HOL;
using namespace HOL::OpenXR;

HandOfLesserCore* HandOfLesserCore::Current = nullptr;

HandOfLesserCore::HandOfLesserCore()
{
}

void HandOfLesserCore::init(int serverPort)
{
	this->Current = this;

	std::string runtimePath = HOL::OpenXR::getActiveOpenXRRuntimePath(1);
	std::string runtimeName = HOL::OpenXR::getActiveOpenXRRuntimeName(1);
	std::cout << "Active OpenXR Runtime is: " << runtimePath << std::endl;

	auto& runtimeState = state::Runtime;
	auto& trackingState = state::Tracking;

	std::memset(runtimeState.runtimeName, 0, sizeof(runtimeState.runtimeName));
	std::strncpy(
		runtimeState.runtimeName, runtimeName.c_str(), sizeof(runtimeState.runtimeName) - 1);
	runtimeState.isVDXR = false;
	runtimeState.isOVR = false;
	runtimeState.openxrState = HOL::OpenXR::OpenXrState::Uninitialized;
	trackingState.isMultimodalEnabled = false;
	trackingState.isHighFidelityEnabled = false;

	std::cout << "OpenXR SDK version: " << xr::Version::current().major() << "."
			  << xr::Version::current().minor() << "." << xr::Version::current().patch()
			  << std::endl;

	if (runtimeName.find("virtualdesktop") != std::string::npos)
	{
		runtimeState.isVDXR = true;
		std::cout << "Runtime is VDXR, will expect being fed lies" << std::endl;
	}

	if (runtimeName.find("oculus_openxr") != std::string::npos)
	{
		runtimeState.isOVR = true;
		std::cout << "Runtime is Oculus, will support multimodal and stuff" << std::endl;
	}

	this->mInstanceHolder.init();
	runtimeState.openxrState = this->mInstanceHolder.getState();

	if (this->mInstanceHolder.getState() != OpenXrState::Failed)
	{
		this->mInstanceHolder.beginSession();

		// Run everything else even if we don't have valid OpenXR so we
		// can test UI and stuff
		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			this->mHandTracking.init(this->mInstanceHolder.mInstance,
									 this->mInstanceHolder.mSession);

			this->mBodyTracking.init(this->mInstanceHolder.mInstance,
									 this->mInstanceHolder.mSession);

			// Airlink doesn't support headless and requires hax
			// As of writing, the only other supported runtime is VDXR
			if (this->mInstanceHolder.fullForegroundMode())
			{
				std::cout << "Running in full foreground mode for testing only!" << std::endl;
			}

			if (runtimeState.isOVR)
			{
				// Hacks to make OVR actually work
				HOL::hacks::fixOvrSessionStateRestriction();
				HOL::hacks::fixOvrMultimodalSupportCheck();
			}
		}
	}

	// Initialize transport as client (connects to driver via named pipe)
	if (!this->mDriverTransport.init(PipeRole::Client, R"(\\.\pipe\HandOfLesser)"))
	{
		std::cerr << "Failed to initialize driver transport" << std::endl;
	}
	else
	{
		// Send initialization packet to notify driver we're connected
		AppInitializedPacket initPacket;
		this->mDriverTransport.send((char*)&initPacket, sizeof(HOL::AppInitializedPacket));
		std::cout << "Sent AppInitialized packet to driver" << std::endl;
	}

	// Initialize OSC transport (send to port 9000, no listening needed)
	if (!this->mOscTransport.init(9000))
	{
		std::cerr << "Failed to initialize OSC transport" << std::endl;
	}

	// Start receive thread for bidirectional communication
	this->mActive = true;
	this->mReceiveThread = std::thread(&HandOfLesserCore::receiveDataThread, this);

	this->featuresManager.setInstanceHolder(&this->mInstanceHolder);
	this->featuresManager.setBodyTracking(&this->mBodyTracking);
	this->syncState();
}

void HandOfLesserCore::start()
{
	this->loadSettings();
	this->mUserInterfaceThread = std::thread(&HandOfLesserCore::userInterfaceLoop, this);
	this->mainLoop();
	this->saveSettings();
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
		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			this->mHandTracking.drawHands();
			this->mBodyTracking.drawBody();
		}
		this->mUserInterface.onFrame();
		if (this->mUserInterface.shouldTerminate())
		{
			break;
		}

		// Apparently it doesn't vsync anymore?
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}

	this->mUserInterface.terminate();
}

void HOL::HandOfLesserCore::receiveDataThread()
{
	std::cout << "App receive thread started" << std::endl;

	bool wasConnected = false;
	bool printedReconnecting = false;

	while (this->mActive)
	{
		// Check if we need to (re)connect
		if (!this->mDriverTransport.isConnected())
		{
			// Only print once when transitioning to disconnected
			if (wasConnected && !printedReconnecting)
			{
				std::cout << "Driver disconnected, attempting to reconnect..." << std::endl;
				printedReconnecting = true;
			}

			wasConnected = false;

			// Close old pipe if any
			this->mDriverTransport.shutdown();

			// Try to reconnect (silently, no spam)
			if (this->mDriverTransport.reconnect())
			{
				// Wait a bit for connection to establish
				for (int i = 0; i < 10 && !this->mDriverTransport.isConnected(); i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}

				if (this->mDriverTransport.isConnected())
				{
					std::cout << "Reconnected to driver!" << std::endl;
					wasConnected = true;
					printedReconnecting = false;
				}
			}

			// If still not connected, wait before retrying
			if (!this->mDriverTransport.isConnected())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
		}
		else
		{
			wasConnected = true;
			printedReconnecting = false;
		}

		HOL::NativePacket* packet = this->mDriverTransport.receivePacket();
		if (packet == nullptr)
		{
			continue; // Timeout, retry
		}

		switch (packet->packetType)
		{
			case NativePacketType::DriverInitialized: {
				auto* initPacket = (DriverInitializedPacket*)packet;
				std::cout << "Driver initialized: version " << initPacket->driverVersion
						  << std::endl;

				// Respond by sending current settings
				syncSettings();
				syncState();
				break;
			}

			case NativePacketType::DriverStatus: {
				auto* statusPacket = (DriverStatusPacket*)packet;
				std::cout << "Driver status: emulated controllers="
						  << statusPacket->emulatedControllersActive
						  << ", hooked=" << statusPacket->hookedControllerCount
						  << ", trackers=" << statusPacket->emulatedTrackerCount << std::endl;
				// Future: update UI status indicators
				break;
			}

			case NativePacketType::DeviceState: {
				auto* devicePacket = (DeviceStatePacket*)packet;

				// Find existing entry or first empty slot
				settings::DeviceConfig* slot = nullptr;
				for (size_t i = 0; i < settings::MAX_DEVICE_CONFIGS; i++)
				{
					if (Config.deviceSettings.devices[i].populated
						&& strcmp(Config.deviceSettings.devices[i].serial, devicePacket->serial)
							   == 0)
					{
						// Found existing entry
						slot = &Config.deviceSettings.devices[i];
						break;
					}
					if (slot == nullptr && !Config.deviceSettings.devices[i].populated)
					{
						// Found empty slot
						slot = &Config.deviceSettings.devices[i];
					}
				}

				if (slot)
				{
					slot->populated = true;
					strncpy_s(slot->serial, sizeof(slot->serial), devicePacket->serial, _TRUNCATE);
					std::cout << "Device registered: " << devicePacket->serial << std::endl;
				}
				else
				{
					std::cerr << "Warning: Device config slots full (max "
							  << settings::MAX_DEVICE_CONFIGS << ")" << std::endl;
				}
				break;
			}

			default:
				std::cerr << "Unknown packet type from driver: " << (int)packet->packetType
						  << std::endl;
		}
	}

	std::cout << "App receive thread stopped" << std::endl;
}

void HandOfLesserCore::mainLoop()
{
	while (1)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		this->mUserInterface.Current->getVisualizer()->clearDrawQueue();

		if (this->mUserInterface.shouldTerminate())
		{
			break;
		}

		if (this->mInstanceHolder.getState() == OpenXrState::Running)
		{
			doOpenXRStuff();
			sendOscData();
		}
		else
		{
			if (Config.vrchat.sendDebugOsc)
			{
				sendOscData();
			}
		}

		// draw queue swapping because UI and main loop are not in sync
		this->mUserInterface.Current->getVisualizer()->swapOuterDrawQueue();

		std::this_thread::sleep_for(std::chrono::milliseconds(Config.general.updateIntervalMS));
	}

	std::cout << "Exiting loop" << std::endl;

	// Signal threads to stop
	this->mActive = false;

	// Wait for threads to finish
	if (this->mUserInterfaceThread.joinable())
	{
		this->mUserInterfaceThread.join();
	}

	if (this->mReceiveThread.joinable())
	{
		this->mReceiveThread.join();
	}
}

void HandOfLesserCore::doOpenXRStuff()
{
	XrTime time = this->mInstanceHolder.getTime();
	time += 1000000LL * (XrTime)Config.general.motionPredictionMS;

	this->mInstanceHolder.pollEvent();

	if (this->mInstanceHolder.fullForegroundMode())
	{
		this->mInstanceHolder.foregroundRender();
	}

	// this->mInstanceHolder.getHmdPosition();
	this->mBodyTracking.updateBody(this->mInstanceHolder.mStageSpace, time);
	this->mHandTracking.updateHands(
		this->mInstanceHolder.mStageSpace, time, this->mBodyTracking.getBodyTracker());
	this->mHandTracking.updateInputs();

	this->sendUpdate();

	return;
}

void HOL::HandOfLesserCore::sendOscData()
{
	if (Config.vrchat.sendDebugOsc)
	{
		this->mVrchatOSC.generateOscTestOutput();
	}
	else
	{
		// Generates the full data which is shared by alternating.
		// Packed requires full to be generated first, and is generated on-demand below.
		this->mVrchatOSC.generateOscOutputFull(
			this->mHandTracking.getHandPose(HandSide::LeftHand),
			this->mHandTracking.getHandPose(HandSide::RightHand));
	}

	size_t size = 0;

	// Always send full, expect when testing remote stuff locally because it will break things
	if (Config.vrchat.sendFull)
	{
		size = this->mVrchatOSC.generateOscBundleFull();
		this->mOscTransport.send(this->mVrchatOSC.getPacketBuffer(), size);
	}

	if (Config.vrchat.sendAlternating)
	{
		size = this->mVrchatOSC.generateOscBundleAlternating();
		this->mOscTransport.send(this->mVrchatOSC.getPacketBuffer(), size);
	}

	if (Config.vrchat.sendPacked)
	{
		// Keep track of values between updates so we don't miss quick movement
		this->mVrchatOSC.handleInbetweenPacked();

		// Wait for 100ms to have passed since last
		if (this->mVrchatOSC.shouldSendPacked())
		{
			this->mVrchatOSC.generateOscOutputPacked();
			size = this->mVrchatOSC.generateOscBundlePacked();
			this->mOscTransport.send(this->mVrchatOSC.getPacketBuffer(), size);
		}
	}

	// VRChat input goes here for now
	// Finalizing also resets the input packet, which will otherwise overflow.
	// Better to control here than having every input check the setting.
	auto [packetPointer, packetSize] = this->mVrchatInput.finalizeInputBundle();
	if (Config.input.sendOscInput)
	{
		this->mOscTransport.send(packetPointer, packetSize);
	}
}

void HandOfLesserCore::sendUpdate()
{

	if (Config.steamvr.sendSteamVRControllerPosition)
	{
		for (int i = 0; i < HandSide_MAX; i++)
		{
			HandPose hand = this->mHandTracking.getHandPose((HandSide)i);

			// No point in sending any new data if the data is the same as last time.
			if (!hand.poseStale)
			{
				HOL::HandTransformPacket transPacket
					= this->mHandTracking.getTransformPacket((HandSide)i);
				this->mDriverTransport.send((char*)&transPacket, sizeof(HOL::HandTransformPacket));

				/*
				HOL::ControllerInputPacket inputPacket
					= this->mHandTracking.getInputPacket((HandSide)i);
				this->mDriverTransport.send((char*)&inputPacket,
				sizeof(HOL::ControllerInputPacket));
				*/
			}
		}
	}

	if (Config.steamvr.sendSteamVRInput)
	{
		// SteamVR inputs are submitted to a global
		for (auto& packet : SteamVR::SteamVRInput::Current->floatInputs)
		{
			this->mDriverTransport.send((char*)&packet, sizeof(HOL::FloatInputPacket));
		}
		for (auto& packet : SteamVR::SteamVRInput::Current->boolInputs)
		{
			this->mDriverTransport.send((char*)&packet, sizeof(HOL::BoolInputPacket));
		}
	}

	if (Config.skeletal.sendSkeletalInput || Config.skeletal.augmentHookedControllers)
	{
		for (int i = 0; i < HandSide_MAX; i++)
		{
			OpenXRHand& hand = this->mHandTracking.getHand((HandSide)i);
			if (hand.handPose.poseValid) // Only update if valid
			{
				SkeletalPacket& packet = this->mSkeletalInput.getSkeletalPacket(hand, (HandSide)i);
				this->mDriverTransport.send((char*)&packet, sizeof(HOL::SkeletalPacket));
			}
		}
	}

	// Send body tracking hand positions for controller detection in driver
	if (state::Tracking.isMultimodalEnabled
		&& Config.handPose.controllerMode != ControllerMode::NoControllerMode)
	{
		MultimodalPosePacket bodyPosePacket = this->mBodyTracking.getMultimodalPosePacket();
		this->mDriverTransport.send((char*)&bodyPosePacket, sizeof(HOL::MultimodalPosePacket));
	}

	// Send body tracker poses
	if (Config.bodyTrackers.enableBodyTrackers)
	{
		sendBodyTrackerData();
	}

	SteamVR::SteamVRInput::Current->clear();
}

void HOL::HandOfLesserCore::sendBodyTrackerData()
{
	// Get body tracker packets from BodyTracking (includes position calculation and visualization)
	auto packets = mBodyTracking.getBodyTrackerPackets();

	// Send each packet
	for (const auto& packet : packets)
	{
		this->mDriverTransport.send((char*)&packet, sizeof(HOL::BodyTrackerPosePacket));
	}
}

void HOL::HandOfLesserCore::syncSettings()
{
	HOL::SettingsPacket packet;
	packet.config = HOL::Config;
	this->mDriverTransport.send((char*)&packet, sizeof(HOL::SettingsPacket));
}

void HOL::HandOfLesserCore::syncState()
{
	state::Runtime.openxrState = this->mInstanceHolder.getState();

	HOL::StatePacket packet;
	packet.tracking = state::Tracking;
	packet.runtime = state::Runtime;
	this->mDriverTransport.send((char*)&packet, sizeof(HOL::StatePacket));
}

void HOL::HandOfLesserCore::saveSettings()
{
	// Good enough for now
	std::ofstream file("settings.json");
	nlohmann::json j = Config;
	file << j.dump(4); // indented
}

void HOL::HandOfLesserCore::loadSettings()
{
	std::ifstream file("settings.json");
	if (file.is_open())
	{
		try
		{
			nlohmann::json j;
			file >> j;
			Config = j.get<HOL::settings::HandOfLesserSettings>();
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Failed to parse settings.json: " << ex.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Failed to parse settings.json due to an unknown error." << std::endl;
		}
		file.close();
	}
	else
	{
		std::cout << "No settings.json found, using defaults." << std::endl;
	}
	syncSettings();
	syncState();
}

bool HOL::HandOfLesserCore::isDriverConnected() const
{
	return this->mDriverTransport.isConnected();
}
