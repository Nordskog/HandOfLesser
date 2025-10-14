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
								xr::Version(1,0,0)}, // VDXR only supports 1.0
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

	if (this->fullForegroundMode())
	{
		setupForegroundRendering();
	}
}

void InstanceHolder::doForegroundRendering(XrFrameState frameState)
{
	XrCompositionLayerProjectionView projectionViews[2] = {
		{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};
	XrCompositionLayerProjection projectionLayer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
	XrCompositionLayerBaseHeader* layers[] = {(XrCompositionLayerBaseHeader*)&projectionLayer};
	XrViewLocateInfo locateInfo = {XR_TYPE_VIEW_LOCATE_INFO};
	locateInfo.displayTime = frameState.predictedDisplayTime;
	locateInfo.space = this->mStageSpace.get();
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	XrViewState viewState = {XR_TYPE_VIEW_STATE};
	XrView views[2] = {{XR_TYPE_VIEW}, {XR_TYPE_VIEW}};
	uint32_t viewCount;
	handleXR("xrLocateViews call",
			 xrLocateViews(this->mSession.get(), &locateInfo, &viewState, 2, &viewCount, views));
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
	// enumerateInstanceExtensionPropertiesToVector() may get stuck if the HMD is not connected properly
	// Observed with airlink, probably means you've been booted back to the menu
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
		XR_FB_BODY_TRACKING_EXTENSION_NAME,
		XR_EXT_HAND_TRACKING_EXTENSION_NAME,
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

void HOL::OpenXR::InstanceHolder::getHmdPosition()
{
	// TODO return actual value. Doesn't work in VD at least.

	XrViewLocateInfo view_locate_info
		= {.type = XR_TYPE_VIEW_LOCATE_INFO,
		   .next = NULL,
		   .viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		   .displayTime = getTime(),
		   .space = this->mStageSpace.get()};

	// TODO: use actual view count
	uint32_t view_count = 2;
	std::vector<XrView> views(view_count);
	for (uint32_t i = 0; i < view_count; i++)
	{
		views[i].type = XR_TYPE_VIEW;
		views[i].next = NULL;
	};

	XrViewState view_state = {.type = XR_TYPE_VIEW_STATE, .next = NULL};
	auto result = xrLocateViews(this->mSession->get(),
								&view_locate_info,
								&view_state,
								view_count,
								&view_count,
								views.data());
	if (handleXR("XrLocateViews", result))
	{
		printf("X: %.3f, Y: %.3f, Z: %.3f\n",
			   views[0].pose.position.x,
			   views[0].pose.position.y,
			   views[0].pose.position.z);

				printf("X: %.3f, Y: %.3f, Z: %.3f\n",
			   views[1].pose.position.x,
			   views[1].pose.position.y,
			   views[1].pose.position.z);
	}
}

void HOL::OpenXR::InstanceHolder::setupForegroundRendering()
{
	///////////////
	// Session
	//////////////

	// Get the viewport configuration info for the chosen viewport configuration type.
	// App only supports the primary stereo view config.
	const XrViewConfigurationType supportedViewConfigType
		= XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	// Enumerate the viewport configurations.
	{
		uint32_t viewportConfigTypeCount = 0;
		handleXR(
			"xrEnumerateViewConfigurations call",
			xrEnumerateViewConfigurations(
				this->mInstance.get(), this->mSystemId.get(), 0, &viewportConfigTypeCount, NULL));

		std::vector<XrViewConfigurationType> viewportConfigurationTypes(viewportConfigTypeCount);

		handleXR("xrEnumerateViewConfigurations call",
				 xrEnumerateViewConfigurations(this->mInstance.get(),
											   this->mSystemId.get(),
											   viewportConfigTypeCount,
											   &viewportConfigTypeCount,
											   viewportConfigurationTypes.data()));

		for (uint32_t i = 0; i < viewportConfigTypeCount; i++)
		{
			const XrViewConfigurationType viewportConfigType = viewportConfigurationTypes[i];
			XrViewConfigurationProperties viewportConfig{XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
			handleXR("xrGetViewConfigurationProperties call",
					 xrGetViewConfigurationProperties(this->mInstance.get(),
													  this->mSystemId.get(),
													  viewportConfigType,
													  &viewportConfig));
			uint32_t viewCount;
			handleXR("xrEnumerateViewConfigurationViews call",
					 xrEnumerateViewConfigurationViews(this->mInstance.get(),
													   this->mSystemId.get(),
													   viewportConfigType,
													   0,
													   &viewCount,
													   NULL));

			if (viewCount > 0)
			{
				std::vector<XrViewConfigurationView> elements(viewCount,
															  {XR_TYPE_VIEW_CONFIGURATION_VIEW});

				handleXR("xrEnumerateViewConfigurationViews call",
						 xrEnumerateViewConfigurationViews(this->mInstance.get(),
														   this->mSystemId.get(),
														   viewportConfigType,
														   viewCount,
														   &viewCount,
														   elements.data()));

				// Cache the view config properties for the selected config type.
				if (viewportConfigType == supportedViewConfigType)
				{
					// assert(viewCount == 2);
					for (uint32_t e = 0; e < viewCount; e++)
					{
						mViewConfigurationView[e] = elements[e];
					}
				}
			}
			else
			{
				printf("Empty viewport configuration type: %d", viewCount);
			}
		}
	}

	// Get the viewport configuration info for the chosen viewport configuration type.
	handleXR("xrGetViewConfigurationProperties call",
			 xrGetViewConfigurationProperties(this->mInstance.get(),
											  this->mSystemId.get(),
											  supportedViewConfigType,
											  &this->mViewportConfiguration));


}

bool HOL::OpenXR::InstanceHolder::fullForegroundMode()
{
	// Set to true to run in foreground when headless mode is not available.
	// Otherwise we will never enter focused mode and must patch oculus dll to get data.
	return !this->isHeadless() && false;
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

int InstanceHolder::foregroundRender()
{
		xr::Time time = getXrTimeNow(this->mInstance.get(), this->mDispatcher);

		if (this->mSessionState != xr::SessionState::Stopping)
		{
			XrFrameWaitInfo waitFrameInfo = {XR_TYPE_FRAME_WAIT_INFO};
			XrFrameState frameState = {XR_TYPE_FRAME_STATE};
			handleXR("xrWaitFrame call",
						xrWaitFrame(this->mSession.get(), &waitFrameInfo, &frameState));

			////////////////
			// Begin frame
			////////////////

			XrFrameBeginInfo beginFrameDesc = {XR_TYPE_FRAME_BEGIN_INFO};
			handleXR("xrBeginFrame call",
						(xrBeginFrame(this->mSession.get(), &beginFrameDesc)));

			doForegroundRendering(frameState);

			///////////////////
			// Frame stuff
			//////////////////////

			XrFrameEndInfo endFrameInfo = {XR_TYPE_FRAME_END_INFO};
			endFrameInfo.displayTime = frameState.predictedDisplayTime;
			endFrameInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
			endFrameInfo.layerCount = 0;
			endFrameInfo.layers = nullptr;

			handleXR("xrEndFrame call", xrEndFrame(this->mSession.get(), &endFrameInfo));
		}

	return 0;
}