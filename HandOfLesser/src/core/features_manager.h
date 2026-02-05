#pragma once

#include <chrono>

namespace HOL
{
	namespace OpenXR
	{
		class InstanceHolder;
		class BodyTracking;
	} // namespace OpenXR

	class FeaturesManager
	{
	public:
		FeaturesManager();

		void setInstanceHolder(OpenXR::InstanceHolder* instanceHolder);
		void setBodyTracking(OpenXR::BodyTracking* bodyTracking);

		void setMultimodalEnabled(bool enabled);
		void requestBodyTrackingFidelity(bool high);

		void applyTrackingFeatures(bool enableHighFidelity, bool enableMultimodal);
		void performPeriodicCheck();

		bool isMultimodalEnabled() const
		{
			return mMultimodalEnabled;
		}

	private:
		OpenXR::InstanceHolder* mInstanceHolder;
		OpenXR::BodyTracking* mBodyTracking;
		bool mMultimodalEnabled = false;

		std::chrono::steady_clock::time_point mLastTrackingFeatureCheckTime;
		const std::chrono::milliseconds TRACKING_FEATURE_CHECK_INTERVAL_MS{5000}; // 5 Seconds
	};
} // namespace HOL
