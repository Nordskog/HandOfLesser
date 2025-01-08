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
	auto colorGrey = IM_COL32(200, 155, 155, 150);
	auto colorWhite = IM_COL32(255, 255, 255, 150);

	auto vis = HOL::UserInterface::Current->getVisualizer();


		XrBodyJointLocationFB* jointLocations = this->mBodyTracker.getLastJointLocations();

		for (int j = 0; j < XR_BODY_JOINT_COUNT_FB; j++)
		{
			XrBodyJointLocationFB& joint = jointLocations[j];

			auto color = colorGrey;
			if (j == XR_BODY_JOINT_LEFT_HAND_WRIST_TWIST_FB)
			{
				color = IM_COL32(255, 0, 0, 255);
			}
		

			// WIll replace this later anyway so nevermind wasteful conversion
			vis->submitPoint(OpenXR::toEigenVector(joint.pose.position), color, 7);
		}
}

OpenXRBody& HOL::OpenXR::BodyTracking::getBodyTracker()
{
	return mBodyTracker;
}


