#include "src/core/hand_of_lesser.h"
#include "hooked_controller.h"
#include "controller_common.h"
#include "src/hooking/hooks.h"
#include <driverlog.h>

namespace HOL
{
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

		// When using multimodal mode on OVR, use position-based detection
		if (HOL::HandOfLesser::Current->mIsOVR && HOL::HandOfLesser::Current->mIsMultimodalEnabled)
		{
			// Get diagnostic info
			HandSide side = this->getSide();
			bool bodyTrackingValid = (side == HandSide::LeftHand)
										 ? HOL::HandOfLesser::Current->mBodyTrackingLeftHandValid
										 : HOL::HandOfLesser::Current->mBodyTrackingRightHandValid;

			// Get distance for logging
			float distance = HOL::HandOfLesser::Current->getControllerToHandDistance(this);
			bool isHeld = HOL::HandOfLesser::Current->isControllerHeldByPosition(this);

			// Possess when controller is NOT held (hands are free)
			bool shouldPossess = !isHeld;

			// Log state changes with diagnostics. TODO: replace with UI display
			if (shouldPossess != mLastPossessionState)
			{
				const char* newState
					= shouldPossess ? "POSSESSING (hands free)" : "RELEASED (controller held)";
				DriverLog("[Multimodal LEFT] %s (distance: %.3fm, threshold: 0.10m, isHeld: "
							"%d, bodyValid: %d, ctrlValid: %d)",
							newState,
							distance,
							isHeld,
							bodyTrackingValid,
							mLastOriginalPoseValid);
				mLastPossessionState = shouldPossess;
			}		

			return shouldPossess;
		}

		// Original logic for non-multimodal mode
		bool canPoss = canPossess();
		if (!mLastOriginalPoseValid && canPoss) // All this needs to be reconsidered.
		{
			// We want to continue possessing while the real controllers
			// are inactive, so it doesn't immediatley jump to their pose instead.
			// This is of course meaningless for VD, since the controllers are always active.
			this->mValidWhileOriginalInvalid = true;
		}

		if (mLastOriginalPoseValid)
		{
			// Only keep posessing while handtracking invalid
			// until original pose becomes valid.
			mValidWhileOriginalInvalid = false;
		}

		// Oculus doesn't send a disconnect signal, they just stop submitting.
		bool originalSubmitStale = framesSinceLastPoseUpdate > 30;

		// As of Quest3, we will have a valid pose despite it being un-tracked.
		// All we care about is whether or not the pose is valid.
		// We should possess anytime the controller is not being tracked.
		// Ideally we would ask OpenXR for this, but we cannot do this properly
		// in headless mode with VDXR, and it also emulates the controllers using handtracking.
		// Our conditions are therefor:
		// # if hands are tracked OR if original pose invalid.
		return this->mLastTransformPacket.tracked
			   || (canPoss && (!mLastOriginalPoseValid || originalSubmitStale))
			   || this->mValidWhileOriginalInvalid;
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

	uint32_t HookedController::getDeviceId()
	{
		return this->mDeviceId;
	}

} // namespace HOL