#include "HandTrackingInterface.h"
#include "XrUtils.h"

// Static members need to be initialized
PFN_xrCreateHandTrackerEXT HandTrackingInterface::xrCreateHandTrackerEXT_ = nullptr;
PFN_xrDestroyHandTrackerEXT HandTrackingInterface::xrDestroyHandTrackerEXT_ = nullptr;
PFN_xrLocateHandJointsEXT HandTrackingInterface::xrLocateHandJointsEXT_ = nullptr;

PFN_xrResumeSimultaneousHandsAndControllersTrackingMETA
	HandTrackingInterface::xrResumEsimultaneousHandsAndControllersTrackingMETA_
	= nullptr;

PFN_xrPauseSimultaneousHandsAndControllersTrackingMETA
	HandTrackingInterface::xrPauseSimultaneousHandsAndControllersTrackingMETA_
	= nullptr;

XrPath LeftHandInteractionPath;
XrPath RightHandInteractionPath;

PFN_xrCreateBodyTrackerFB HandTrackingInterface::xrCreateBodyTrackerFB_ = nullptr;
PFN_xrDestroyBodyTrackerFB HandTrackingInterface::xrDestroyBodyTrackerFB_ = nullptr;
PFN_xrLocateBodyJointsFB HandTrackingInterface::xrLocateBodyJointsFB_ = nullptr;

using namespace HOL::OpenXR;

void HandTrackingInterface::init(xr::UniqueDynamicInstance& instance)
{
	initFunctions(instance);
}

void HandTrackingInterface::initFunctions(xr::UniqueDynamicInstance& instance)
{
	XrInstance inst = instance.get();

	// Hand
	handleXR("xrCreateHandTrackerEXT get",
			 xrGetInstanceProcAddr(
				 inst, "xrCreateHandTrackerEXT", (PFN_xrVoidFunction*)(&xrCreateHandTrackerEXT_)));

	handleXR("xrDestroyHandTrackerEXT get",
			 xrGetInstanceProcAddr(inst,
								   "xrDestroyHandTrackerEXT",
								   (PFN_xrVoidFunction*)(&xrDestroyHandTrackerEXT_)));

	handleXR("xrLocateHandJointsEXT get",
			 xrGetInstanceProcAddr(
				 inst, "xrLocateHandJointsEXT", (PFN_xrVoidFunction*)(&xrLocateHandJointsEXT_)));

	// Body
	handleXR("xrCreateBodyTrackerFB get",
			 xrGetInstanceProcAddr(
				 inst, "xrCreateBodyTrackerFB", (PFN_xrVoidFunction*)(&xrCreateBodyTrackerFB_)));

	handleXR("xrDestroyBodyTrackerFB get",
			 xrGetInstanceProcAddr(
				 inst, "xrDestroyBodyTrackerFB", (PFN_xrVoidFunction*)(&xrDestroyBodyTrackerFB_)));

	handleXR("xrLocateBodyJointsFB get",
			 xrGetInstanceProcAddr(
				 inst, "xrLocateBodyJointsFB", (PFN_xrVoidFunction*)(&xrLocateBodyJointsFB_)));

	// Multimodal
	handleXR("xrResumEsimultaneousHandsAndControllersTrackingMETA get",
			 xrGetInstanceProcAddr(
				 inst,
				 "xrResumeSimultaneousHandsAndControllersTrackingMETA",
				 (PFN_xrVoidFunction*)(&xrResumEsimultaneousHandsAndControllersTrackingMETA_)));

	handleXR("xrPauseSimultaneousHandsAndControllersTrackingMETA get",
			 xrGetInstanceProcAddr(
				 inst,
				 "xrPauseSimultaneousHandsAndControllersTrackingMETA",
				 (PFN_xrVoidFunction*)(&xrPauseSimultaneousHandsAndControllersTrackingMETA_)));
}

void HandTrackingInterface::createHandTracker(xr::UniqueDynamicSession& session,
											  XrHandEXT side,
											  XrHandTrackerEXT& handTrackerOut)
{
	XrHandTrackerCreateInfoEXT createInfo{XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT};
	createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
	createInfo.hand = side;
	handleXR("xrCreateHandTrackerEXT_ call",
			 xrCreateHandTrackerEXT_(session.get(), &createInfo, &handTrackerOut));
}

bool HandTrackingInterface::locateHandJoints(XrHandTrackerEXT& handTracker,
											 xr::UniqueDynamicSpace& space,
											 XrTime time,
											 XrHandJointLocationEXT* handJointLocationsOut,
											 XrHandJointVelocityEXT* handJointVelocitiesOut)
{

	XrHandJointVelocitiesEXT velocities{XR_TYPE_HAND_JOINT_VELOCITIES_EXT};
	velocities.next = NULL;
	velocities.jointCount = XR_HAND_JOINT_COUNT_EXT;
	velocities.jointVelocities = handJointVelocitiesOut;

	XrHandJointLocationsEXT locations{XR_TYPE_HAND_JOINT_LOCATIONS_EXT};
	locations.next = &velocities;
	locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
	locations.jointLocations = handJointLocationsOut;

	XrHandJointsLocateInfoEXT locateInfo{XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT};
	locateInfo.baseSpace = space.get();
	locateInfo.time = time;

	handleXR("xrLocateHandJointsEXT_ call",
			 xrLocateHandJointsEXT_(handTracker, &locateInfo, &locations));

	return locations.isActive;
}

void HandTrackingInterface::destroyHandTracker()
{
	// TODO
}

void HandTrackingInterface::createBodyTracker(xr::UniqueDynamicSession& session,
											  XrBodyTrackerFB& bodyTrackerOut)
{
	XrBodyTrackerCreateInfoFB createInfo{XR_TYPE_BODY_TRACKER_CREATE_INFO_FB};
	createInfo.bodyJointSet = XR_BODY_JOINT_SET_DEFAULT_FB;
	createInfo.next = nullptr;
	handleXR("xrCreateBodyTrackerFB_ call",
			 xrCreateBodyTrackerFB_(session.get(), &createInfo, &bodyTrackerOut));
}

void HandTrackingInterface::destroyBodyTracker()
{
}

float HandTrackingInterface::locateBodyJoints(XrBodyTrackerFB& bodyTracker,
											  xr::UniqueDynamicSpace& space,
											  XrTime time,
											  XrBodyJointLocationFB* bodyJointLocationsOut)
{
	XrBodyJointLocationsFB locations{XR_TYPE_BODY_JOINT_LOCATIONS_FB};
	locations.next = NULL;
	locations.jointCount = XR_BODY_JOINT_COUNT_FB;
	locations.jointLocations = bodyJointLocationsOut;

	XrBodyJointsLocateInfoFB locateInfo{XR_TYPE_BODY_JOINTS_LOCATE_INFO_FB};
	locateInfo.baseSpace = space.get();
	locateInfo.time = time;

	handleXR("xrLocateHandJointsEXT_ call",
			 xrLocateBodyJointsFB_(bodyTracker, &locateInfo, &locations));

	return locations.confidence;
}

void HandTrackingInterface::resumeMultimodal(xr::UniqueDynamicSession& session)
{
	XrSimultaneousHandsAndControllersTrackingResumeInfoMETA resumeInfo{
		XR_TYPE_SIMULTANEOUS_HANDS_AND_CONTROLLERS_TRACKING_RESUME_INFO_META};

	resumeInfo.next = nullptr;

	handleXR("XrSystemSimultaneousHandsAndControllersPropertiesMETA_ call",
			 xrResumEsimultaneousHandsAndControllersTrackingMETA_(session.get(), &resumeInfo));
}

void HandTrackingInterface::pauseMultimodal(xr::UniqueDynamicSession& session)
{
	XrSimultaneousHandsAndControllersTrackingPauseInfoMETA pauseInfo{
		XR_TYPE_SIMULTANEOUS_HANDS_AND_CONTROLLERS_TRACKING_PAUSE_INFO_META};

	pauseInfo.next = nullptr;

	handleXR("xrPauseSimultaneousHandsAndControllersTrackingMETA call",
			 xrPauseSimultaneousHandsAndControllersTrackingMETA_(session.get(), &pauseInfo));
}
