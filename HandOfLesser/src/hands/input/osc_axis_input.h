#pragma once

#include "base_input.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "src/vrchat/vrchat_input.h"

namespace HOL
{
	class OscAxisInput : public BaseInput<Eigen::Vector2f>
	{
	public:
		static std::shared_ptr<OscAxisInput> Create()
		{
			return std::make_shared<OscAxisInput>();
		}

		void submit(Eigen::Vector2f inputData) override;

	private:
		// This stuff should be moved to a common VRC input thing
		std::string mXInput = "Horizontal";
		std::string mYInput = "Vertical";
	};
} // namespace HOL