#pragma once

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

		void enableMultimodal();
		void disableMultimodal();

		void requestHighFidelity();
		void requestLowFidelity();

	private:
		OpenXR::InstanceHolder* mInstanceHolder;
		OpenXR::BodyTracking* mBodyTracking;
	};
} // namespace HOL
