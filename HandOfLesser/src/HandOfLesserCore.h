#pragma once

#include <memory>
#include "InstanceHolder.h"
#include "HandTracking.h"
#include "XrEventsInterface.h"

class HandOfLesserCore : public XrEventsInterface
{
	public:
		void init();
		void start();

		virtual std::vector<const char*> getRequiredExtensions();
		virtual void onFrame( XrTime time );


	private:
		std::unique_ptr<InstanceHolder> mInstanceHolder;
		std::unique_ptr<HandTracking> mHandTracking;
};