#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/vrchat/vrchat_input.h"

namespace HOL
{
	class OscFloatInput : public BaseInput<float>, public std::enable_shared_from_this<OscFloatInput>
	{
	public:
		static std::shared_ptr<OscFloatInput> Create()
		{
			return std::make_shared<OscFloatInput>();
		}

		std::shared_ptr<OscFloatInput> setup(std::string input);

		void submit(float inputData) override;

	private:
		// This stuff should be moved to a common VRC input thing
		std::string mInput = "";
	};
} // namespace HOL