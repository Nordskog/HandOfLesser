#pragma once

#include <exception>

namespace HOL::Windows
{
	class CrashHandler
	{
	public:
		static void install();
		static void logHandledException(const std::exception& ex);
		static void logHandledUnknownException();
	};
} // namespace HOL::Windows
