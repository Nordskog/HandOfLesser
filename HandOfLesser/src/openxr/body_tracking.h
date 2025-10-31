#pragma once

#include <d3d11.h> // Why do you need this??
#include <memory>
#include "openxr_body.h" // replace with body!
#include <HandOfLesserCommon.h>

namespace HOL::OpenXR
{
	class BodyTracking
	{
	public:
		void init(xr::UniqueDynamicInstance& instance, xr::UniqueDynamicSession& session);
		void updateBody(xr::UniqueDynamicSpace& space, XrTime time);
		void drawBody();
		OpenXRBody& getBodyTracker();
		HOL::MultimodalPosePacket getMultimodalPosePacket();
		std::vector<HOL::BodyTrackerPosePacket> getBodyTrackerPackets();

	private:
		OpenXRBody mBodyTracker;
	};
} // namespace HOL::OpenXR
