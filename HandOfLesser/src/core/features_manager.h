#pragma once

namespace HOL
{
	namespace OpenXR
	{
		class InstanceHolder;
	}

	class FeaturesManager
	{
	public:
		FeaturesManager();

		void setInstanceHolder(OpenXR::InstanceHolder* instanceHolder);
		void enableMultimodal();
		void disableMultimodal();

	private:
		OpenXR::InstanceHolder* mInstanceHolder;
	};
} // namespace HOL
