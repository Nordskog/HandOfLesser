#include "openxr_body.h"
#include "HandTrackingInterface.h"

void OpenXRBody::init(xr::UniqueDynamicSession& session)
{
	HandTrackingInterface::createBodyTracker(session, mBodyTracker);
}

void OpenXRBody::updateJointLocations(xr::UniqueDynamicSpace& space, XrTime time)
{
	HandTrackingInterface::locateBodyJoints(
		this->mBodyTracker, space, time, this->mJointLocations);
}

XrBodyJointLocationFB* OpenXRBody::getLastJointLocations()
{
	return this->mJointLocations;
}
