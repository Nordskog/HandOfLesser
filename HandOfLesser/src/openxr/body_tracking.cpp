#include "body_tracking.h"
#include "src/core/ui/user_interface.h"
#include "XrUtils.h"

void HOL::OpenXR::BodyTracking::init(xr::UniqueDynamicInstance& instance,
									 xr::UniqueDynamicSession& session)
{
	this->mBodyTracker.init(session);
}

void HOL::OpenXR::BodyTracking::updateBody(xr::UniqueDynamicSpace& space, XrTime time)
{
	this->mBodyTracker.updateJointLocations(space, time);
}

void HOL::OpenXR::BodyTracking::drawBody()
{
	auto colorTracked = IM_COL32(0, 255, 0, 150);	 // Green semi-transparent for tracked
	auto colorNotTracked = IM_COL32(255, 0, 0, 150); // Red semi-transparent for not tracked

	auto vis = HOL::UserInterface::Current->getVisualizer();

	XrBodyJointLocationFB* jointLocations = this->mBodyTracker.getLastJointLocations();

	for (int j = 0; j < XR_BODY_JOINT_COUNT_FB; j++)
	{
		XrBodyJointLocationFB& joint = jointLocations[j];

		// Check if joint is tracked
		bool isTracked = (joint.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)
						 == XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

		auto color = isTracked ? colorTracked : colorNotTracked;

		// WIll replace this later anyway so nevermind wasteful conversion
		vis->submitPoint(OpenXR::toEigenVector(joint.pose.position), color, 7);
	}
}

OpenXRBody& HOL::OpenXR::BodyTracking::getBodyTracker()
{
	return mBodyTracker;
}
