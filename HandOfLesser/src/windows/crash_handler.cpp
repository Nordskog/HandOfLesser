#include "crash_handler.h"
#include "src/core/app_paths.h"
#include "windows_utils.h"
#include <dbghelp.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

namespace
{
	constexpr int MaxStackFrames = 32;

	const char* getStructuredExceptionName(DWORD exceptionCode)
	{
		switch (exceptionCode)
		{
			case EXCEPTION_ACCESS_VIOLATION:
				return "EXCEPTION_ACCESS_VIOLATION";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
			case EXCEPTION_BREAKPOINT:
				return "EXCEPTION_BREAKPOINT";
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				return "EXCEPTION_DATATYPE_MISALIGNMENT";
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				return "EXCEPTION_FLT_DENORMAL_OPERAND";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
			case EXCEPTION_FLT_INVALID_OPERATION:
				return "EXCEPTION_FLT_INVALID_OPERATION";
			case EXCEPTION_FLT_OVERFLOW:
				return "EXCEPTION_FLT_OVERFLOW";
			case EXCEPTION_FLT_STACK_CHECK:
				return "EXCEPTION_FLT_STACK_CHECK";
			case EXCEPTION_FLT_UNDERFLOW:
				return "EXCEPTION_FLT_UNDERFLOW";
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				return "EXCEPTION_ILLEGAL_INSTRUCTION";
			case EXCEPTION_IN_PAGE_ERROR:
				return "EXCEPTION_IN_PAGE_ERROR";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				return "EXCEPTION_INT_DIVIDE_BY_ZERO";
			case EXCEPTION_INT_OVERFLOW:
				return "EXCEPTION_INT_OVERFLOW";
			case EXCEPTION_INVALID_DISPOSITION:
				return "EXCEPTION_INVALID_DISPOSITION";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
			case EXCEPTION_PRIV_INSTRUCTION:
				return "EXCEPTION_PRIV_INSTRUCTION";
			case EXCEPTION_SINGLE_STEP:
				return "EXCEPTION_SINGLE_STEP";
			case EXCEPTION_STACK_OVERFLOW:
				return "EXCEPTION_STACK_OVERFLOW";
			default:
				return "UNKNOWN_STRUCTURED_EXCEPTION";
		}
	}

	bool initializeSymbols()
	{
		static bool initialized = false;
		static bool attempted = false;
		if (attempted)
		{
			return initialized;
		}

		attempted = true;
		DWORD options = SymGetOptions();
		options |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS;
		SymSetOptions(options);
		initialized = SymInitialize(GetCurrentProcess(), nullptr, TRUE) == TRUE;
		return initialized;
	}

	std::string formatAddress(void* address)
	{
		std::ostringstream out;
		out << address;

		if (!initializeSymbols())
		{
			return out.str();
		}

		DWORD64 displacement = 0;
		char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		if (SymFromAddr(
				GetCurrentProcess(), reinterpret_cast<DWORD64>(address), &displacement, symbol))
		{
			out << " (" << symbol->Name;
			if (displacement != 0)
			{
				out << " +0x" << std::hex << displacement << std::dec;
			}
			out << ")";
		}

		IMAGEHLP_LINE64 lineInfo{};
		lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		DWORD lineDisplacement = 0;
		if (SymGetLineFromAddr64(GetCurrentProcess(),
								 reinterpret_cast<DWORD64>(address),
								 &lineDisplacement,
								 &lineInfo))
		{
			out << " [" << lineInfo.FileName << ":" << lineInfo.LineNumber << "]";
		}

		return out.str();
	}

	void printStackFrames(void* const* frames, USHORT frameCount)
	{
		if (frameCount == 0)
		{
			return;
		}

		std::cerr << "Stack trace:" << std::endl;
		for (USHORT i = 0; i < frameCount; i++)
		{
			std::cerr << "  [" << i << "] " << formatAddress(frames[i]) << std::endl;
		}
	}

	void printCurrentStackTrace()
	{
		void* frames[MaxStackFrames]{};
		USHORT frameCount = CaptureStackBackTrace(0, MaxStackFrames, frames, nullptr);
		printStackFrames(frames, frameCount);
	}

	USHORT captureExceptionStackTrace(EXCEPTION_POINTERS* exceptionPointers,
									  void** frames,
									  USHORT maxFrames)
	{
		if (exceptionPointers == nullptr || exceptionPointers->ContextRecord == nullptr
			|| frames == nullptr || maxFrames == 0)
		{
			return 0;
		}

		CONTEXT context = *exceptionPointers->ContextRecord;
		STACKFRAME64 stackFrame{};
		DWORD machineType = 0;

#if defined(_M_X64)
		machineType = IMAGE_FILE_MACHINE_AMD64;
		stackFrame.AddrPC.Offset = context.Rip;
		stackFrame.AddrFrame.Offset = context.Rbp;
		stackFrame.AddrStack.Offset = context.Rsp;
#elif defined(_M_IX86)
		machineType = IMAGE_FILE_MACHINE_I386;
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrFrame.Offset = context.Ebp;
		stackFrame.AddrStack.Offset = context.Esp;
#else
		return 0;
#endif

		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Mode = AddrModeFlat;

		HANDLE process = GetCurrentProcess();
		HANDLE thread = GetCurrentThread();
		USHORT frameCount = 0;
		while (frameCount < maxFrames)
		{
			if (!StackWalk64(machineType,
							 process,
							 thread,
							 &stackFrame,
							 &context,
							 nullptr,
							 SymFunctionTableAccess64,
							 SymGetModuleBase64,
							 nullptr))
			{
				break;
			}

			if (stackFrame.AddrPC.Offset == 0)
			{
				break;
			}

			frames[frameCount++] = reinterpret_cast<void*>(stackFrame.AddrPC.Offset);
		}

		return frameCount;
	}

	std::filesystem::path writeMiniDump(EXCEPTION_POINTERS* exceptionPointers)
	{
		std::filesystem::path crashDirectory = HOL::Paths::getCrashDirectory();
		std::error_code errorCode;
		std::filesystem::create_directories(crashDirectory, errorCode);

		SYSTEMTIME localTime{};
		GetLocalTime(&localTime);

		char dumpFileName[128]{};
		std::snprintf(dumpFileName,
					  sizeof(dumpFileName),
					  "crash_%04d%02d%02d_%02d%02d%02d.dmp",
					  localTime.wYear,
					  localTime.wMonth,
					  localTime.wDay,
					  localTime.wHour,
					  localTime.wMinute,
					  localTime.wSecond);

		std::filesystem::path dumpPath = crashDirectory / dumpFileName;
		HANDLE dumpFile = CreateFileA(dumpPath.string().c_str(),
									  GENERIC_WRITE,
									  0,
									  nullptr,
									  CREATE_ALWAYS,
									  FILE_ATTRIBUTE_NORMAL,
									  nullptr);
		if (dumpFile == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Failed to create crash dump file: " << FormatWindowsError(GetLastError())
					  << std::endl;
			return {};
		}

		MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo{};
		dumpExceptionInfo.ThreadId = GetCurrentThreadId();
		dumpExceptionInfo.ExceptionPointers = exceptionPointers;
		dumpExceptionInfo.ClientPointers = FALSE;

		BOOL wroteDump = MiniDumpWriteDump(GetCurrentProcess(),
										   GetCurrentProcessId(),
										   dumpFile,
										   MiniDumpNormal,
										   exceptionPointers ? &dumpExceptionInfo : nullptr,
										   nullptr,
										   nullptr);
		CloseHandle(dumpFile);
		if (!wroteDump)
		{
			std::cerr << "Failed to write crash dump: " << FormatWindowsError(GetLastError())
					  << std::endl;
			return {};
		}

		return dumpPath;
	}

	LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
	{
		if (exceptionPointers == nullptr || exceptionPointers->ExceptionRecord == nullptr)
		{
			std::cerr << "Fatal structured exception: no exception record available" << std::endl;
			printCurrentStackTrace();
			return EXCEPTION_EXECUTE_HANDLER;
		}

		const EXCEPTION_RECORD* record = exceptionPointers->ExceptionRecord;
		std::cerr << "Fatal structured exception: "
				  << getStructuredExceptionName(record->ExceptionCode) << " ("
				  << FormatWindowsError(record->ExceptionCode) << ")" << std::endl;
		std::cerr << "Exception address: " << formatAddress(record->ExceptionAddress) << std::endl;

		if ((record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
			 || record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
			&& record->NumberParameters >= 2)
		{
			const char* accessType = "unknown";
			if (record->ExceptionInformation[0] == 0)
			{
				accessType = "read";
			}
			else if (record->ExceptionInformation[0] == 1)
			{
				accessType = "write";
			}
			else if (record->ExceptionInformation[0] == 8)
			{
				accessType = "execute";
			}

			std::cerr << "Access violation type: " << accessType << std::endl;
			std::cerr << "Fault address: "
					  << formatAddress(reinterpret_cast<void*>(record->ExceptionInformation[1]))
					  << std::endl;
		}

		void* frames[MaxStackFrames]{};
		USHORT frameCount = captureExceptionStackTrace(exceptionPointers, frames, MaxStackFrames);
		if (frameCount > 0)
		{
			printStackFrames(frames, frameCount);
		}
		else
		{
			printCurrentStackTrace();
		}

		std::filesystem::path dumpPath = writeMiniDump(exceptionPointers);
		if (!dumpPath.empty())
		{
			std::cerr << "Crash dump written to: " << dumpPath.string() << std::endl;
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}

	void terminateHandler()
	{
		std::cerr << "Fatal error: std::terminate called" << std::endl;

		if (std::exception_ptr currentException = std::current_exception())
		{
			try
			{
				std::rethrow_exception(currentException);
			}
			catch (const std::exception& ex)
			{
				std::cerr << "Unhandled C++ exception: " << ex.what() << std::endl;
			}
			catch (const std::string& message)
			{
				std::cerr << "Unhandled C++ exception: " << message << std::endl;
			}
			catch (const char* message)
			{
				std::cerr << "Unhandled C++ exception: " << message << std::endl;
			}
			catch (...)
			{
				std::cerr << "Unhandled C++ exception of unknown type" << std::endl;
			}
		}

		printCurrentStackTrace();
		std::_Exit(1);
	}
} // namespace

void HOL::Windows::CrashHandler::install()
{
	initializeSymbols();
	SetUnhandledExceptionFilter(unhandledExceptionFilter);
	std::set_terminate(terminateHandler);
}

void HOL::Windows::CrashHandler::logHandledException(const std::exception& ex)
{
	initializeSymbols();
	std::cerr << "Fatal handled C++ exception: " << ex.what() << std::endl;
	printCurrentStackTrace();
}

void HOL::Windows::CrashHandler::logHandledUnknownException()
{
	initializeSymbols();
	std::cerr << "Fatal handled C++ exception: unknown type" << std::endl;
	printCurrentStackTrace();
}
