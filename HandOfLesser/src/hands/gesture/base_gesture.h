#pragma once

#include <d3d11.h> // Why do you need this??
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <vector>
#include <functional>
#include <HandOfLesserCommon.h>
#include <memory>

namespace HOL
{
	struct GestureData
	{
		XrHandJointLocationEXT* joints[HandSide::HandSide_MAX];
		XrPosef HMDPose;
	};

	class BaseGesture
	{
	public:
		BaseGesture(){};
		static std::shared_ptr<BaseGesture> Create()
		{
			return std::make_shared<BaseGesture>();
		}

		float evaluate(GestureData data);

		std::vector<std::shared_ptr<BaseGesture>>& getSubGestures();

		float lastValue = 0;
		std::string name = "baseGesture";
		std::vector<std::shared_ptr<BaseGesture>> mSubGestures;

	protected:
		virtual float evaluateInternal(GestureData data)
		{
			return 0;
		}
	};
} // namespace HOL
