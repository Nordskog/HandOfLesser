#include "InstanceHolder.h"

#include <iostream>
#include "XrUtils.h"
#include <thread>

InstanceHolder::InstanceHolder()
{
    this->mCallback = nullptr;
}

void InstanceHolder::init()
{
	this->mDispatcher = xr::DispatchLoaderDynamic{};

    this->enumerateLayers();
    this->enumerateExtensions();
    this->initExtensions();

    this->initInstance();
    this->initSession();
    this->initSpaces();
}

int InstanceHolder::mainLoop()
{
    int frameNum = 0;

    while (1)
    {
        ++frameNum;
        pollEvent(this->mInstance.get(), this->mDispatcher);
        xr::Time time = getXrTimeNow(this->mInstance.get(), this->mDispatcher);

        //std::cout << "Iteration: " << frameNum << "\n";

        if (this->mCallback != nullptr)
        {
            //std::cout << "Will call callback" << std::endl;
            this->mCallback->onFrame(time.get());
        }

        // Must manually sleep since we aren't waiting on a frame.
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

    }

    this->mSession->requestExitSession(this->mDispatcher);
    this->mSession->endSession(this->mDispatcher);

    return 0;
}

void InstanceHolder::initInstance()
{
    // We don't enable any 
    std::vector<const char*> enabledLayers;

    this->mInstance = xr::createInstanceUnique(
        xr::InstanceCreateInfo{
        xr::InstanceCreateFlagBits::None,
            xr::ApplicationInfo{"HandOfLesser", 1, // app version
            "", 0,                     // engine version
            xr::Version::current()},
            uint32_t(enabledLayers.size()),
            enabledLayers.data(),
            uint32_t(this->mEnabledExtensions.size()),
            this->mEnabledExtensions.data(),
    },
        this->mDispatcher);

    // Update the dispatch now that we have an instance
    this->mDispatcher = xr::DispatchLoaderDynamic::createFullyPopulated(this->mInstance.get(), &::xrGetInstanceProcAddr);

    pollEvent(this->mInstance.get(), this->mDispatcher);
}

void InstanceHolder::initSession()
{
    this->mSystemId = this->mInstance->getSystem(xr::SystemGetInfo{xr::FormFactor::HeadMountedDisplay}, this->mDispatcher);

    xr::SessionCreateInfo createInfo{xr::SessionCreateFlagBits::None, this->mSystemId};
    xr::GraphicsBindingD3D11KHR D3D11Binding;
    D3D11Binding.type = xr::StructureType::GraphicsBindingD3D11KHR;

    auto requirements = this->mInstance->getD3D11GraphicsRequirementsKHR(this->mSystemId, this->mDispatcher);
    createInfo.next = get(D3D11Binding);

    ID3D11Device* Device;
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0x00, NULL, 0, D3D11_SDK_VERSION, &Device, NULL, NULL);
    D3D11Binding.device = Device;

    createInfo.systemId = this->mSystemId;

    this->mSession = this->mInstance->createSessionUnique(createInfo, this->mDispatcher);

    pollEvent(this->mInstance.get(), this->mDispatcher);
}

void InstanceHolder::initSpaces()
{
    this->mLocalSpace = this->mSession->createReferenceSpaceUnique(
        xr::ReferenceSpaceCreateInfo{xr::ReferenceSpaceType::Local, xr::Posef{}}, this->mDispatcher);

    mStageSpace = this->mSession->createReferenceSpaceUnique(
        xr::ReferenceSpaceCreateInfo{xr::ReferenceSpaceType::Stage, xr::Posef{}}, this->mDispatcher);
}

void InstanceHolder::beginSession()
{
    auto viewConfigType = xr::ViewConfigurationType::PrimaryStereo;

    // Begin session
    this->mSession->beginSession({ viewConfigType }, this->mDispatcher);

    pollEvent(this->mInstance.get(), this->mDispatcher);
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
    this->mExtensions = xr::enumerateInstanceExtensionPropertiesToVector(nullptr, this->mDispatcher);
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
     XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME,    // Gestures
     XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME,   // Used to get current time.
     XR_KHR_D3D11_ENABLE_EXTENSION_NAME
    };

    for (auto& extension : this->mEnabledExtensions)
    {
        if ( !hasExtension(this->mExtensions, extension) )
        {
            std::cout << "Missing required extensions: " << extension << std::endl;
        }
    }
}

void InstanceHolder::setCallback(XrEventsInterface* callback)
{
    this->mCallback = callback;
}

