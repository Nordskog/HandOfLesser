#pragma once

#include <memory>
#include "InstanceHolder.h"
#include "HandTracking.h"
#include "XrEventsInterface.h"
#include <HandOfLesserCommon.h>

class HandOfLesserCore : public XrEventsInterface
{
	public:
		void init(int serverPort);
		void start();

		virtual std::vector<const char*> getRequiredExtensions();
		virtual void onFrame( XrTime time );


	private:
		std::unique_ptr<InstanceHolder> mInstanceHolder;
		std::unique_ptr<HandTracking> mHandTracking;
		HOL::NativeTransport mTransport;

		void sendUpdate();
};