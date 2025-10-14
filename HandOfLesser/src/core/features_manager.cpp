#include "features_manager.h"
#include "src/openxr/HandTrackingInterface.h"
#include "src/openxr/InstanceHolder.h"

namespace HOL
{
	FeaturesManager::FeaturesManager()
		: mInstanceHolder(nullptr)
	{
	}

	void FeaturesManager::setInstanceHolder(OpenXR::InstanceHolder* instanceHolder)
	{
		mInstanceHolder = instanceHolder;
	}

	void FeaturesManager::enableMultimodal()
	{
		if (mInstanceHolder && mInstanceHolder->mSession)
		{
			HandTrackingInterface::resumeMultimodal(mInstanceHolder->mSession);
		}
	}

	void FeaturesManager::disableMultimodal()
	{
		if (mInstanceHolder && mInstanceHolder->mSession)
		{
			HandTrackingInterface::pauseMultimodal(mInstanceHolder->mSession);
		}
	}
} // namespace HOL
