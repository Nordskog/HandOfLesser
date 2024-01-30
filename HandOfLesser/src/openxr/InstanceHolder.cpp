#include "InstanceHolder.h"

#include <iostream>
#include "XrUtils.h"
#include <thread>
#include "src/core/ui/display_global.h"

using namespace HOL::OpenXR;

InstanceHolder::InstanceHolder()
{
	this->mState = OpenXrState::Uninitialized;
	this->mCallback = nullptr;
}

void InstanceHolder::init()
{
	this->mDispatcher = xr::DispatchLoaderDynamic{};

	try
	{
		this->enumerateLayers();
		this->enumerateExtensions();
		this->initExtensions();

		this->initInstance();
		this->initSession();
		this->initSpaces();
	}
	catch (std::exception exp)
	{
		std::cerr << exp.what() << std::endl;
		std::cerr << "Failed to initialize OpenXR" << std::endl;
		this->updateState(OpenXrState::Failed);
		return;
	}

	this->updateState(OpenXrState::Initialized);
}

void InstanceHolder::initInstance()
{
	// We don't enable any
	std::vector<const char*> enabledLayers;

	this->mInstance = xr::createInstanceUnique(
		xr::InstanceCreateInfo{
			xr::InstanceCreateFlagBits::None,
			xr::ApplicationInfo{"HandOfLesser",
								1, // app version
								"",
								0, // engine version
								xr::Version::current()},
			uint32_t(enabledLayers.size()),
			enabledLayers.data(),
			uint32_t(this->mEnabledExtensions.size()),
			this->mEnabledExtensions.data(),
		},
		this->mDispatcher);

	// Update the dispatch now that we have an instance
	this->mDispatcher = xr::DispatchLoaderDynamic::createFullyPopulated(this->mInstance.get(),
																		&::xrGetInstanceProcAddr);

	pollEvent();
}

void InstanceHolder::initSession()
{
	this->mSystemId = this->mInstance->getSystem(
		xr::SystemGetInfo{xr::FormFactor::HeadMountedDisplay}, this->mDispatcher);

	xr::SessionCreateInfo createInfo{xr::SessionCreateFlagBits::None, this->mSystemId};

	// Non-headless requires that we at least pretend we're going to display stuff
	if (!this->isHeadless())
	{
		xr::GraphicsBindingD3D11KHR D3D11Binding;
		D3D11Binding.type = xr::StructureType::GraphicsBindingD3D11KHR;

		auto requirements
			= this->mInstance->getD3D11GraphicsRequirementsKHR(this->mSystemId, this->mDispatcher);
		createInfo.next = get(D3D11Binding);

		ID3D11Device* Device;
		D3D11CreateDevice(NULL,
						  D3D_DRIVER_TYPE_HARDWARE,
						  NULL,
						  0x00,
						  NULL,
						  0,
						  D3D11_SDK_VERSION,
						  &Device,
						  NULL,
						  NULL);
		D3D11Binding.device = Device;
	}

	createInfo.systemId = this->mSystemId;

	this->mSession = this->mInstance->createSessionUnique(createInfo, this->mDispatcher);

	pollEvent();
}

void InstanceHolder::initSpaces()
{
	this->mLocalSpace = this->mSession->createReferenceSpaceUnique(
		xr::ReferenceSpaceCreateInfo{xr::ReferenceSpaceType::Local, xr::Posef{}},
		this->mDispatcher);

	mStageSpace = this->mSession->createReferenceSpaceUnique(
		xr::ReferenceSpaceCreateInfo{xr::ReferenceSpaceType::Stage, xr::Posef{}},
		this->mDispatcher);
}

void InstanceHolder::pollEvent()
{
	pollEventInternal(this->mInstance.get(), this->mDispatcher);
}

void InstanceHolder::endSession()
{
	if (mState == OpenXrState::Running)
	{
		this->mSession->requestExitSession(this->mDispatcher);
		this->mSession->endSession(this->mDispatcher);
		this->updateState(OpenXrState::Exited);
	}
}

void InstanceHolder::beginSession()
{

	// Ignored if headless
	auto viewConfigType = xr::ViewConfigurationType::PrimaryStereo;

	try
	{
		// Begin session
		this->mSession->beginSession({viewConfigType}, this->mDispatcher);
	}
	catch (std::exception exp)
	{
		std::cerr << exp.what() << std::endl;
		std::cerr << "Failed to begin OpenXR session" << std::endl;
		this->updateState(OpenXrState::Failed);
		return;
	}

	this->pollEvent();
	this->updateState(OpenXrState::Running);
}

void InstanceHolder::enumerateLayers()
{
	this->mLayers = xr::enumerateApiLayerPropertiesToVector(this->mDispatcher);
	std::cout << "Enumerating layers:" << std::endl;
	std::cout << "Number of layers: " << this->mLayers.size() << std::endl;
	for (const auto& prop : this->mLayers)
	{
		std::cout << prop.layerName << " - " << prop.description << std::endl;
	}
}

void InstanceHolder::enumerateExtensions()
{
	this->mExtensions
		= xr::enumerateInstanceExtensionPropertiesToVector(nullptr, this->mDispatcher);
	std::cout << "Enumerating extensions:" << std::endl;
	std::cout << "Number of extensions: " << this->mExtensions.size() << std::endl;
	for (auto& extension : this->mExtensions)
	{
		std::cout << extension.extensionName << std::endl;
	}
}

void InstanceHolder::initExtensions()
{
	this->mEnabledExtensions = {
		XR_EXT_HAND_TRACKING_EXTENSION_NAME,
		XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME,						  // Gestures
		XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME, // Used to get current time.
		XR_KHR_D3D11_ENABLE_EXTENSION_NAME};

	if (hasExtension(this->mExtensions, XR_MND_HEADLESS_EXTENSION_NAME))
	{
		std::cout << "Runtime supports headless extension, running in headless mode" << std::endl;
		this->mEnabledExtensions.push_back(XR_MND_HEADLESS_EXTENSION_NAME);
		this->mHeadless = true;
	}
	else
	{
		std::cout << "Runtime does not support headless extension, running as foreground app" << std::endl;
		this->mHeadless = false;
	}

	for (auto& extension : this->mEnabledExtensions)
	{
		if (!hasExtension(this->mExtensions, extension))
		{
			std::cout << "Missing required extensions: " << extension << std::endl;
		}
	}
}

void InstanceHolder::setCallback(XrEventsInterface* callback)
{
	this->mCallback = callback;
}

XrTime InstanceHolder::getTime()
{
	return getXrTimeNow(this->mInstance.get(), this->mDispatcher).get();
}

void InstanceHolder::updateState(OpenXrState newState)
{
	this->mState = newState;
	HOL::display::OpenXrInstanceState = newState;
}

OpenXrState InstanceHolder::getState()
{
	return this->mState;
}

bool HOL::OpenXR::InstanceHolder::isHeadless()
{
	return this->mHeadless;
}
