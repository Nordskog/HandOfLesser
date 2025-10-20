#pragma once

#include <HandOfLesserCommon.h>
#include "openvr_driver.h"
#include <random>

namespace HOL::ControllerCommon
{
	extern vr::DriverPose_t generatePose(HOL::HandTransformPacket* packet, bool deviceConnected);

	extern vr::DriverPose_t generateDisconnectedPose();

	extern vr::DriverPose_t addJitter(const vr::DriverPose_t& existingPose);

	extern void offsetPose(vr::DriverPose_t& existingPose,
						   HOL::HandSide side,
						   Eigen::Vector3f translationOffset,
						   Eigen::Vector3f rotationOffset);

	extern void applyDriverOffset(Eigen::Vector3f& position,
											 Eigen::Quaternionf& rotation,
											 const vr::DriverPose_t& pose);

	vr::VRBoneTransform_t poseLocationToBoneTransform(HOL::PoseLocation& location);

} // namespace HOL::ControllerCommon
