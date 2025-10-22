#include "src/core/hand_of_lesser.h"
#include "hooked_controller.h"
#include "controller_common.h"
#include "src/hooking/hooks.h"
#include <driverlog.h>
#include <src/utils/math_utils.h>

namespace HOL
{
	void HookedController::clearAugmentedSkeleton()
	{
		mSkeletonHandle = 0;
	}

	void HookedController::resetPossessionHints()
	{
		mValidWhileOriginalInvalid = false;
		mLastPossessionState = false;
	}

	bool HookedController::isSuppressed() const
	{
		return mSuppressed;
	}

	void HookedController::setSuppressed(bool suppressed)
	{
		if (mSuppressed == suppressed)
		{
			return;
		}

		mSuppressed = suppressed;

		if (suppressed)
		{
			vr::DriverPose_t disconnectPose = HOL::ControllerCommon::generateDisconnectedPose();
			HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				this->mHookedHost, this->mDeviceId, disconnectPose, sizeof(vr::DriverPose_t));
		}
	}

	HookedController::HookedController(uint32_t id,
									   HandSide side,
									   vr::IVRServerDriverHost* host,
									   vr::ITrackedDeviceServerDriver* driver,
									   vr::PropertyContainerHandle_t propertyContainer)
	{
		this->mSide = side;
		this->mDeviceId = id;
		this->mHookedHost = host;
		this->mHookedDriver = driver;
		this->propertyContainer = propertyContainer;
	}

	void HookedController::lateInit(std::string serial,
									vr::ETrackedDeviceClass deviceClass,
									vr::ETrackedControllerRole role)
	{
		this->serial = serial;
		this->mDeviceClass = deviceClass;
		this->role = role;
	}

	void HookedController::registerSkeletonInput(vr::VRInputComponentHandle_t handle,
												 vr::EVRSkeletalTrackingLevel level,
												 const std::string& path)
	{
		mSkeletonHandle = handle;
		mSkeletonTrackingLevel = level;
		mSkeletonInputPath = path;
		mLoggedMissingSkeletonHandle = false;
		DriverLog(
			"Hooked controller %s registered skeleton input %s", serial.c_str(), path.c_str());
	}

	void HookedController::UpdatePose(HOL::HandTransformPacket* packet)
	{
		// packet data resides in receive buffer and will be replaced on next receive,
		// so make a copy now.
		this->mLastTransformPacket = *packet;

		// Do not update pose if invalid, because we want to continue submitting
		// the last valid one. Is this necessary? is there some kind of timeout?
		if (this->mLastTransformPacket.valid)
		{
			this->mLastPose = ControllerCommon::generatePose(&this->mLastTransformPacket, true);
		}
	}
	void HookedController::UpdateInput(HOL::ControllerInputPacket* packet)
	{
	}
	void HookedController::UpdateBoolInput(const std::string& input, bool value)
	{
		auto inputHandle = this->inputHandlesByName.find(input);
		if (inputHandle != this->inputHandlesByName.end())
		{
			hooks::UpdateBooleanComponent::FunctionHook.originalFunc(
				this->driverInput, (*inputHandle).second, value, 0.0);
		}
	}

	void HookedController::UpdateFloatInput(const std::string& input, float value)
	{
		auto inputHandle = this->inputHandlesByName.find(input);
		if (inputHandle != this->inputHandlesByName.end())
		{
			hooks::UpdateScalarComponent::FunctionHook.originalFunc(
				this->driverInput, (*inputHandle).second, value, 0.0);
		}
	}

	void HookedController::UpdateSkeletal(HOL::SkeletalPacket* packet)
	{
		auto& config = HOL::HandOfLesser::Current->Config;
		const auto& trackingState = HOL::HandOfLesser::Tracking;

		if (!config.skeletal.augmentHookedControllers)
		{
			clearAugmentedSkeleton();
			return;
		}

		if (!trackingState.isMultimodalEnabled)
		{
			clearAugmentedSkeleton();
			return;
		}

		if (packet == nullptr || packet->side != mSide)
		{
			clearAugmentedSkeleton();
			return;
		}

		if (this->driverInput == nullptr)
		{
			clearAugmentedSkeleton();
			return;
		}

		if (mSkeletonHandle == 0)
		{
			for (auto& [handle, input] : inputHandles)
			{
				if (input.type == ControllerInputType::Skeleton)
				{
					mSkeletonHandle = handle;
					mSkeletonInputPath = input.inputPath;
					break;
				}
			}
		}

		if (mSkeletonHandle == 0)
		{
			if (!mLoggedMissingSkeletonHandle)
			{
				DriverLog("Hooked controller %s missing skeleton handle", serial.c_str());
				mLoggedMissingSkeletonHandle = true;
			}
			clearAugmentedSkeleton();
			return;
		}

		mLoggedMissingSkeletonHandle = false;

		HOL::ControllerCommon::buildSkeletalPoseFromPacket(*packet, mSkeletalPose);

		if (hooks::UpdateSkeletonComponent::FunctionHook.originalFunc != nullptr)
		{
			hooks::UpdateSkeletonComponent::FunctionHook.originalFunc(
				this->driverInput,
				mSkeletonHandle,
				vr::VRSkeletalMotionRange_WithoutController,
				mSkeletalPose,
				SteamVR::HandSkeletonBone::eBone_Count);
		}
		else
		{
			this->driverInput->UpdateSkeletonComponent(mSkeletonHandle,
													   vr::VRSkeletalMotionRange_WithoutController,
													   mSkeletalPose,
													   SteamVR::HandSkeletonBone::eBone_Count);
		}
	}

	bool HookedController::isAugmentedSkeletonActive() const
	{
		const auto& config = HOL::HandOfLesser::Current->Config;
		if (!config.skeletal.augmentHookedControllers)
		{
			return false;
		}

		if (!HOL::HandOfLesser::Tracking.isMultimodalEnabled)
		{
			return false;
		}

		if (this->driverInput == nullptr || mSkeletonHandle == 0)
		{
			return false;
		}

		return true;
	}

	void HookedController::SubmitPose()
	{
		auto& config = HOL::HandOfLesser::Current->Config;

		if (HOL::HandOfLesser::Current->shouldPossess(this))
		{
			// In this state we prevent the original hooked call from running,
			// and call the original function with our pose instead.

			// If we are submitting a stale pose to lock it in place, we must jitter it
			// because vrchat is stupid and ignores all the status information steamvr provides.
			const auto& pose
				= (this->mLastTransformPacket.valid || !config.steamvr.jitterLastPoseOnTrackingLoss)
					  ? this->mLastPose
					  : HOL::ControllerCommon::addJitter(this->mLastPose);

			HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
				this->mHookedHost, this->mDeviceId, pose, sizeof(vr::DriverPose_t));
		}
		else if (config.handPose.controllerMode == ControllerMode::OffsetControllerMode)
		{
			// If we have gonte 5 frames without a pose update when offsetting, we should also
			// jitter the pose so vrchat doesn't disable our arms for several seconds.
			if (config.steamvr.jitterLastPoseOnTrackingLoss && this->framesSinceLastPoseUpdate > 5)
			{
				const auto& pose = HOL::ControllerCommon::addJitter(this->lastOriginalPose);

				HOL::hooks::TrackedDevicePoseUpdated::FunctionHook.originalFunc(
					this->mHookedHost, this->mDeviceId, pose, sizeof(vr::DriverPose_t));
			}
		}
	}

	bool HookedController::isHeld()
	{
		const float HELD_THRESHOLD = 0.15f; // 10cm

		// TODO: Require a few consecutive frames to make switch
		float distance = HOL::HandOfLesser::Current->getControllerToHandDistance(this);

		if (distance > 99999) // Invalid
		{
			// If we don't know, we don't know.
			// Retain current value.
			return mLastHeldState;
		}

		mLastHeldState = distance < HELD_THRESHOLD;

		return mLastHeldState;
	}

	// Can, not should.
	bool HookedController::canPossess()
	{
		// Only controllers should ever be possessed - never HMD or other tracked devices
		if (mDeviceClass != vr::TrackedDeviceClass_Controller)
		{
			return false;
		}

		// Whether or not the pose is valid pretty much.
		return this->mLastTransformPacket.valid;
	}

	// Assuming other external conditions also say it should.
	bool HookedController::shouldPossess()
	{
		// Only controllers should ever be possessed - never HMD or other tracked devices
		if (mDeviceClass != vr::TrackedDeviceClass_Controller)
		{
			return false;
		}

		return HOL::HandOfLesser::Current->isHandTrackingPrimary(this->mSide);
	}

	void HookedController::setSide(HandSide side)
	{
		this->mSide = side;
	}

	HandSide HookedController::getSide()
	{
		return mSide;
	}

	void HookedController::updateSideFromRole()
	{
		// Devices that do not have a pre-determined left/right role
		// also have no way of telling which side they've been assigned to.
		// Best you can do is ask SteamVR what the current controller for whichever side
		// is, and check if that corresponds to any given controller.

		// Get the role
		auto props = vr::VRProperties();
		vr::PropertyContainerHandle_t container
			= props->TrackedDeviceToPropertyContainer(this->mDeviceId);

		// Now we can get the role ( except for vive wands maybe )
		vr::ETrackedControllerRole role = (vr::ETrackedControllerRole)props->GetInt32Property(
			container, vr::Prop_ControllerRoleHint_Int32);

		HandSide side = HandSide::HandSide_MAX;
		switch (role)
		{
			case vr::ETrackedControllerRole::TrackedControllerRole_LeftHand:
				side = HandSide::LeftHand;
				break;
			case vr::ETrackedControllerRole::TrackedControllerRole_RightHand:
				side = HandSide::RightHand;
				break;
			default:
				side = HandSide::HandSide_MAX;
		}

		if (mSide != side)
		{
			if (side == HandSide_MAX)
			{
				DriverLog("Controller unassigned side somehow.");
			}
			else
			{
				DriverLog("Controller assigned new side: %s",
						  side == HandSide::LeftHand ? "Left" : "Right");
			}
		}
	}

	void HookedController::setLastOriginalPoseState(bool valid)
	{
		this->mLastOriginalPoseValid = valid;
	}

	Eigen::Vector3f HookedController::getWorldPosition()
	{
		// The position in the pose is before driver offsets have been applied.
		// Apply them to get the world position so we can more easily compare it.
		Eigen::Vector3f position = HOL::ovrVectorToEigen(lastOriginalPose.vecPosition);
		Eigen::Quaternionf rotation = HOL::ovrQuaternionToEigen(lastOriginalPose.qRotation);

		// TODO: Check pose validity?
		ControllerCommon::applyDriverOffset(position, rotation, mLastPose);

		return position;
	}

	uint32_t HookedController::getDeviceId()
	{
		return this->mDeviceId;
	}

} // namespace HOL
