#pragma once

#include "display_global.h"

namespace HOL::display
{
	HandTransformDisplay HandTransform[2];
	OpenXR::OpenXrState OpenXrInstanceState = OpenXR::OpenXrState::Uninitialized;
} // namespace HOL::display
