#include "HandTrackingInterface.h"
#include "XrUtils.h"

// Static members need to be initialized
PFN_xrCreateHandTrackerEXT HandTrackingInterface::xrCreateHandTrackerEXT_ = nullptr;
PFN_xrDestroyHandTrackerEXT HandTrackingInterface::xrDestroyHandTrackerEXT_ = nullptr;
PFN_xrLocateHandJointsEXT HandTrackingInterface::xrLocateHandJointsEXT_ = nullptr;

void HandTrackingInterface::init(xr::UniqueDynamicInstance& instance)
{
    initFunctions( instance );
}

void HandTrackingInterface::initFunctions(xr::UniqueDynamicInstance& instance )
{
    XrInstance inst = instance.get();

    handleXR(
        "xrCreateHandTrackerEXT get", 
        xrGetInstanceProcAddr(inst,
        "xrCreateHandTrackerEXT",
        (PFN_xrVoidFunction*)(&xrCreateHandTrackerEXT_)));

    handleXR(
        "xrDestroyHandTrackerEXT get", 
        xrGetInstanceProcAddr(inst,
        "xrDestroyHandTrackerEXT",
        (PFN_xrVoidFunction*)(&xrDestroyHandTrackerEXT_))
    );

    handleXR(
        "xrLocateHandJointsEXT get",
        xrGetInstanceProcAddr(inst,
        "xrLocateHandJointsEXT", 
        (PFN_xrVoidFunction*)(&xrLocateHandJointsEXT_))
    );

}

void HandTrackingInterface::createHandTracker( xr::UniqueDynamicSession& session, XrHandEXT side, XrHandTrackerEXT& handTrackerOut)
{
    XrHandTrackerCreateInfoEXT createInfo{ XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
    createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
    createInfo.hand = side;
    handleXR("xrCreateHandTrackerEXT_ call", xrCreateHandTrackerEXT_(session.get(), &createInfo, &handTrackerOut));
}

void HandTrackingInterface::locateHandJoints
(
    XrHandTrackerEXT& handTracker,
    xr::UniqueDynamicSpace& space, 
    XrTime time, 
    XrHandJointLocationEXT* handJointLocationsOut,
    XrHandJointVelocityEXT* handJointVelocitiesOut,
    XrHandTrackingAimStateFB* aimStateOut
)
{
    // Data is returned directly to this struct
    aimStateOut->type = XR_TYPE_HAND_TRACKING_AIM_STATE_FB;
    aimStateOut->next = NULL;

    XrHandJointVelocitiesEXT velocities{ XR_TYPE_HAND_JOINT_VELOCITIES_EXT };
    velocities.next = aimStateOut; 
    velocities.jointCount = XR_HAND_JOINT_COUNT_EXT;
    velocities.jointVelocities = handJointVelocitiesOut;

    XrHandJointLocationsEXT locations{ XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
    locations.next = &velocities;
    locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
    locations.jointLocations = handJointLocationsOut;

    XrHandJointsLocateInfoEXT locateInfo{ XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
    locateInfo.baseSpace = space.get();
    locateInfo.time = time;

    handleXR("xrLocateHandJointsEXT_ call",
        xrLocateHandJointsEXT_(handTracker, &locateInfo, &locations));


}

void HandTrackingInterface::destroyHandTracker()
{
    // TODO


}