#pragma once

#include <d3d11.h> // Needs to go before all the openxr stuff 
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include "XrEventsInterface.h"

#include <iostream>

class InstanceHolder
{
	public:
        InstanceHolder();
        void init();
        void beginSession();
        void pollEvent();
        void endSession();
        void setCallback(XrEventsInterface* callback);
        XrTime getTime();

        xr::UniqueDynamicInstance mInstance;
        xr::UniqueDynamicSession mSession;
        xr::UniqueDynamicSpace mLocalSpace;
        xr::UniqueDynamicSpace mStageSpace;
        xr::DispatchLoaderDynamic mDispatcher;

	private:

        xr::SystemId mSystemId;

		std::vector<xr::ExtensionProperties> mExtensions;
		std::vector<xr::ApiLayerProperties> mLayers;
		std::vector<const char*> mEnabledExtensions;

        XrEventsInterface* mCallback;

		void enumerateLayers();
		void enumerateExtensions();
		void initExtensions();

		void initInstance();
        void initSession();
        void initSpaces();

        template <typename Dispatch> void pollEventInternal(xr::Instance instance, Dispatch&& d)
        {
            while (1)
            {
                xr::EventDataBuffer event;
                auto result = instance.pollEvent(event, d);
                if (result == xr::Result::EventUnavailable)
                {
                    return;
                }
                else if (result != xr::Result::Success)
                {
                    std::cout << "Got error polling for events: " << to_string(result) << std::endl;
                    return;
                }

                std::cout << "Event: " << to_string_literal(event.type) << std::endl;
                if (event.type == xr::StructureType::EventDataSessionStateChanged)
                {
                    auto& change = reinterpret_cast<xr::EventDataSessionStateChanged&>(event);
                    std::cout << to_string(change.state) << std::endl;
                }
            }
        }
};