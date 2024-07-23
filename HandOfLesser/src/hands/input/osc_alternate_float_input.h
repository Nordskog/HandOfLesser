#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/vrchat/vrchat_input.h"

namespace HOL
{
	class OscAlternateFloatInput : public BaseInput<float>
	{
	public:
		static std::shared_ptr<OscAlternateFloatInput> Create()
		{
			return std::make_shared<OscAlternateFloatInput>();
		}

		void setup(std::string inputOn, std::string inputOff);

		void submit(float inputData) override;

	private:
		// This stuff should be moved to a common VRC input thing
		std::string mInputOn = "";
		std::string mInputOff = "";
		bool mBinary = false;
	};
} // namespace HOL