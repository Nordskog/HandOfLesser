#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>
#include <limits>

namespace HOL
{
	class SteamVRFloatInput : public BaseInput<float>,
							  public std::enable_shared_from_this<SteamVRFloatInput>
	{
	public:
		static std::shared_ptr<SteamVRFloatInput> Create()
		{
			return std::make_shared<SteamVRFloatInput>();
		}

		std::shared_ptr<SteamVRFloatInput> setup(HOL::HandSide side, const std::string input);

		void submit(float inputData) override;

	private:
		// This stuff should be moved to a common VRC input thing
		std::string mInput = "";
		HOL::HandSide mSide = HOL::HandSide::LeftHand;
		float mLastValue = std::numeric_limits<float>::max();
	};
} // namespace HOL