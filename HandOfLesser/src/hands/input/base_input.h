#pragma once

#include <memory>
#include <chrono>

namespace HOL
{

	template <typename T> class BaseInput
	{
	public:
		BaseInput(){};
		static std::shared_ptr<BaseInput> Create()
		{
			return std::make_shared<BaseInput>();
		}

		virtual void submit(T inputData)
		{
			return;
		}
	};

} // namespace HOL