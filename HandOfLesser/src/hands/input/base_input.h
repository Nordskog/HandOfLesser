#pragma once

#include <memory>
#include <chrono>

namespace HOL
{
	enum InputType
	{
		Touch, Button, Trigger, XAxis, YAxis, ZAxis, InputType_MAX
	};

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