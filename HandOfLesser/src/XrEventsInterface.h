#pragma once

#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

class XrEventsInterface
{
public:
	virtual std::vector<const char*> getRequiredExtensions() = 0;
	virtual bool onFrame( XrTime time ) = 0;
};