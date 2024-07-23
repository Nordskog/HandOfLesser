#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <HandOfLesserCommon.h>
#include <limits>

namespace HOL
{
	class SteamVRBoolInput : public BaseInput<float>,
							 public std::enable_shared_from_this<SteamVRBoolInput>
	{
	public:
		static std::shared_ptr<SteamVRBoolInput> Create()
		{
			return std::make_shared<SteamVRBoolInput>();
		}

		std::shared_ptr<SteamVRBoolInput> setup(HOL::HandSide side, const std::string input);

		void submit(float inputData) override;

	private:
		// This stuff should be moved to a common VRC input thing
		std::string mInput = "";
		HOL::HandSide mSide = HOL::HandSide::LeftHand;
		bool mLastValue = false;	// Ideally should be undefined, but good enough.
	};
} // namespace HOL