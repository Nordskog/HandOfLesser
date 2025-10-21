#include "features_manager.h"
#include "src/openxr/HandTrackingInterface.h"
#include "src/openxr/InstanceHolder.h"
#include "src/openxr/body_tracking.h"
#include "src/core/HandOfLesserCore.h"
#include "src/core/state_global.h"

namespace HOL
{
	FeaturesManager::FeaturesManager() : mInstanceHolder(nullptr), mBodyTracking(nullptr)
	{
	}

	void FeaturesManager::setInstanceHolder(OpenXR::InstanceHolder* instanceHolder)
	{
		mInstanceHolder = instanceHolder;
	}

	void FeaturesManager::setBodyTracking(OpenXR::BodyTracking* bodyTracking)
	{
		mBodyTracking = bodyTracking;
	}

	void FeaturesManager::enableMultimodal()
	{
		if (mInstanceHolder && mInstanceHolder->mSession)
		{
			HandTrackingInterface::resumeMultimodal(mInstanceHolder->mSession);
			mMultimodalEnabled = true;
			state::Tracking.isMultimodalEnabled = true;
			HandOfLesserCore::Current->syncState();
		}
	}

	void FeaturesManager::disableMultimodal()
	{
		if (mInstanceHolder && mInstanceHolder->mSession)
		{
			HandTrackingInterface::pauseMultimodal(mInstanceHolder->mSession);
			mMultimodalEnabled = false;
			state::Tracking.isMultimodalEnabled = false;
			HandOfLesserCore::Current->syncState();
		}
	}

	void FeaturesManager::requestHighFidelity()
	{
		if (mBodyTracking)
		{
			XrBodyTrackerFB bodyTracker = mBodyTracking->getBodyTracker().getBodyTrackerFB();
			HandTrackingInterface::requestBodyTrackingFidelity(bodyTracker,
															   XR_BODY_TRACKING_FIDELITY_HIGH_META);
			state::Tracking.isHighFidelityEnabled = true;
			HandOfLesserCore::Current->syncState();
		}
	}

	void FeaturesManager::requestLowFidelity()
	{
		if (mBodyTracking)
		{
			XrBodyTrackerFB bodyTracker = mBodyTracking->getBodyTracker().getBodyTrackerFB();
			HandTrackingInterface::requestBodyTrackingFidelity(bodyTracker,
															   XR_BODY_TRACKING_FIDELITY_LOW_META);
			state::Tracking.isHighFidelityEnabled = false;
			HandOfLesserCore::Current->syncState();
		}
	}
} // namespace HOL
