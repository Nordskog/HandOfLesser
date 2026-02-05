#include "features_manager.h"
#include "src/openxr/HandTrackingInterface.h"
#include "src/openxr/InstanceHolder.h"
#include "src/openxr/body_tracking.h"
#include "src/core/HandOfLesserCore.h"
#include "src/core/state_global.h"
#include "src/core/settings_global.h"

namespace HOL
{
	FeaturesManager::FeaturesManager() : mInstanceHolder(nullptr), mBodyTracking(nullptr)
	{
		mLastTrackingFeatureCheckTime = std::chrono::steady_clock::now();
	}

	void FeaturesManager::setInstanceHolder(OpenXR::InstanceHolder* instanceHolder)
	{
		mInstanceHolder = instanceHolder;
	}

	void FeaturesManager::setBodyTracking(OpenXR::BodyTracking* bodyTracking)
	{
		mBodyTracking = bodyTracking;
	}

	void FeaturesManager::setMultimodalEnabled(bool enabled)
	{
		if (mInstanceHolder && mInstanceHolder->mSession)
		{
			if (enabled)
			{
				HandTrackingInterface::resumeMultimodal(mInstanceHolder->mSession);
				if (!mMultimodalEnabled)
				{
					mMultimodalEnabled = true;
					state::Tracking.isMultimodalEnabled = true;
					HandOfLesserCore::Current->syncState();
				}
			}
			else
			{
				HandTrackingInterface::pauseMultimodal(mInstanceHolder->mSession);
				if (mMultimodalEnabled)
				{
					mMultimodalEnabled = false;
					state::Tracking.isMultimodalEnabled = false;
					HandOfLesserCore::Current->syncState();
				}
			}
		}
	}

	void FeaturesManager::requestBodyTrackingFidelity(bool high)
	{
		if (mBodyTracking)
		{
			XrBodyTrackerFB bodyTracker = mBodyTracking->getBodyTracker().getBodyTrackerFB();

			if (high)
			{
				HandTrackingInterface::requestBodyTrackingFidelity(
					bodyTracker, XR_BODY_TRACKING_FIDELITY_HIGH_META);
				if (!state::Tracking.isHighFidelityEnabled)
				{
					state::Tracking.isHighFidelityEnabled = true;
					HandOfLesserCore::Current->syncState();
				}
			}
			else
			{
				HandTrackingInterface::requestBodyTrackingFidelity(
					bodyTracker, XR_BODY_TRACKING_FIDELITY_LOW_META);
				if (state::Tracking.isHighFidelityEnabled)
				{
					state::Tracking.isHighFidelityEnabled = false;
					HandOfLesserCore::Current->syncState();
				}
			}
		}
	}

	void FeaturesManager::applyTrackingFeatures(bool enableHighFidelity, bool enableMultimodal)
	{
		// Always disable multimodal first to avoid conflicts
		if (mMultimodalEnabled)
			this->setMultimodalEnabled(false);

		// Enable high fidelity if requested
		this->requestBodyTrackingFidelity(enableHighFidelity);

		// Re-enable multimodal if requested
		if (enableMultimodal)
			this->setMultimodalEnabled(true);
	}
} // namespace HOL

void FeaturesManager::performPeriodicCheck()
{
	auto now = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now
															  - this->mLastTrackingFeatureCheckTime)
		>= TRACKING_FEATURE_CHECK_INTERVAL_MS)
	{
		// Re-request High Fidelity if enabled
		if (Config.trackingFeatures.enableUpperBodyTracking)
		{
			requestBodyTrackingFidelity(true);
		}

		// Re-enable Multimodal if enabled
		if (Config.trackingFeatures.enableSimultaneousTracking)
		{
			setMultimodalEnabled(true);
		}

		this->mLastTrackingFeatureCheckTime = now;
	}
} // namespace HOL
