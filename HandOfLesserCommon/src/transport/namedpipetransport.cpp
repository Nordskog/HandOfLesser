#include "namedpipetransport.h"
#include "transportutil.h"
#include <iostream>
#include <cstring>

using namespace HOL;

NamedPipeTransport::~NamedPipeTransport()
{
	shutdown();
}

bool NamedPipeTransport::init(PipeRole role, const char* pipeName)
{
	mRole = role;
	strncpy_s(mPipeName, sizeof(mPipeName), pipeName, _TRUNCATE);

	if (mRole == PipeRole::Server)
	{
		return createServerPipe();
	}
	else
	{
		return connectClientPipe();
	}
}

void NamedPipeTransport::shutdown()
{
	cleanup();
}

bool NamedPipeTransport::reconnect()
{
	shutdown();
	return init(mRole, mPipeName);
}

bool NamedPipeTransport::createServerPipe()
{
	// Create the named pipe
	mPipe = CreateNamedPipeA(mPipeName,
							 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // Bidirectional, async
							 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE
								 | PIPE_WAIT, // Message mode, blocking
							 1,				  // Max instances (only 1 client allowed)
							 8192,			  // Out buffer size
							 8192,			  // In buffer size
							 0,				  // Default timeout
							 nullptr);		  // Default security

	if (mPipe == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::cerr << "Failed to create named pipe: " << error << std::endl;
		return false; // Permanent error
	}

	// Create event handles for overlapped I/O
	mReadOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	mWriteOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (!mReadOverlapped.hEvent || !mWriteOverlapped.hEvent)
	{
		std::cerr << "Failed to create event handles" << std::endl;
		cleanup();
		return false;
	}

	std::cout << "Named pipe server created: " << mPipeName << std::endl;
	return true;
}

bool NamedPipeTransport::connectClientPipe()
{
	// Check if pipe exists (short timeout)
	if (!WaitNamedPipeA(mPipeName, 100)) // 100ms timeout
	{
		// No pipe, will retry
		return true;
	}

	// Pipe exists, try to connect
	mPipe = CreateFileA(mPipeName,
						GENERIC_READ | GENERIC_WRITE,
						0,			   // No sharing
						nullptr,	   // Default security
						OPEN_EXISTING, // Pipe must exist
						FILE_FLAG_OVERLAPPED,
						nullptr);

	if (mPipe == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		if (isPermanentError(error))
		{
			std::cerr << "Permanent error connecting to pipe: " << error << std::endl;
			return false;
		}

		// Transient error - we'll retry later silently
		return true;
	}

	// Set message read mode
	DWORD mode = PIPE_READMODE_MESSAGE;
	if (!SetNamedPipeHandleState(mPipe, &mode, nullptr, nullptr))
	{
		DWORD error = GetLastError();
		std::cerr << "Failed to set pipe mode: " << error << std::endl;
		CloseHandle(mPipe);
		mPipe = INVALID_HANDLE_VALUE;
		return false;
	}

	// Create event handles for overlapped I/O
	mReadOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	mWriteOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (!mReadOverlapped.hEvent || !mWriteOverlapped.hEvent)
	{
		std::cerr << "Failed to create event handles" << std::endl;
		cleanup();
		return false;
	}

	mConnected = true;
	std::cout << "Connected to named pipe: " << mPipeName << std::endl;
	return true;
}

bool NamedPipeTransport::waitForConnection(DWORD timeoutMs)
{
	if (mRole == PipeRole::Client)
	{
		// Client already connected during init
		return mConnected;
	}

	// Server side - wait for client to connect
	if (mConnected)
	{
		return true; // Already connected
	}

	ResetEvent(mReadOverlapped.hEvent);

	BOOL result = ConnectNamedPipe(mPipe, &mReadOverlapped);

	if (result)
	{
		// Connected immediately (shouldn't happen with overlapped)
		mConnected = true;
		return true;
	}

	DWORD error = GetLastError();

	if (error == ERROR_PIPE_CONNECTED)
	{
		// Client already connected
		mConnected = true;
		return true;
	}

	if (error == ERROR_IO_PENDING)
	{
		// Wait for connection with timeout
		DWORD waitResult = WaitForSingleObject(mReadOverlapped.hEvent, timeoutMs);

		if (waitResult == WAIT_OBJECT_0)
		{
			// Check if connection completed
			DWORD bytesTransferred;
			if (GetOverlappedResult(mPipe, &mReadOverlapped, &bytesTransferred, FALSE))
			{
				mConnected = true;
				return true;
			}
		}
		else if (waitResult == WAIT_TIMEOUT)
		{
			// Timeout - cancel pending operation
			CancelIo(mPipe);
			return false;
		}
	}

	return false;
}

size_t NamedPipeTransport::send(const char* buffer, size_t size)
{
	if (!mConnected || mPipe == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD bytesWritten = 0;
	ResetEvent(mWriteOverlapped.hEvent);

	BOOL success = WriteFile(mPipe, buffer, (DWORD)size, &bytesWritten, &mWriteOverlapped);

	if (success)
	{
		// Completed immediately
		return bytesWritten;
	}

	DWORD error = GetLastError();

	if (error == ERROR_IO_PENDING)
	{
		// Wait for completion (1 second timeout)
		DWORD waitResult = WaitForSingleObject(mWriteOverlapped.hEvent, 1000);

		if (waitResult == WAIT_OBJECT_0)
		{
			if (GetOverlappedResult(mPipe, &mWriteOverlapped, &bytesWritten, FALSE))
			{
				return bytesWritten;
			}
		}
		else
		{
			// Timeout or error - cancel operation
			CancelIo(mPipe);
		}
	}

	// Write failed - mark disconnected
	std::cerr << "Pipe write failed: " << GetLastError() << std::endl;
	mConnected = false;
	return 0;
}

size_t NamedPipeTransport::receive(char* buffer, size_t maxSize)
{
	DWORD timeoutMs = 1000;

	if (mPipe == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// If not connected and we're the server, try to accept connection
	if (!mConnected && mRole == PipeRole::Server)
	{
		if (!waitForConnection(timeoutMs))
		{
			return 0; // Timeout waiting for client
		}
	}

	if (!mConnected)
	{
		return 0;
	}

	DWORD bytesRead = 0;
	ResetEvent(mReadOverlapped.hEvent);

	BOOL success = ReadFile(mPipe, buffer, (DWORD)maxSize, &bytesRead, &mReadOverlapped);

	if (success)
	{
		// Completed immediately
		return bytesRead;
	}

	DWORD error = GetLastError();

	if (error == ERROR_IO_PENDING)
	{
		// Wait for completion with timeout
		DWORD waitResult = WaitForSingleObject(mReadOverlapped.hEvent, timeoutMs);

		if (waitResult == WAIT_OBJECT_0)
		{
			if (GetOverlappedResult(mPipe, &mReadOverlapped, &bytesRead, FALSE))
			{
				return bytesRead;
			}

			error = GetLastError();
		}
		else if (waitResult == WAIT_TIMEOUT)
		{
			// Timeout - cancel pending operation and return 0
			CancelIo(mPipe);
			return 0;
		}
		else
		{
			// Wait failed
			CancelIo(mPipe);
			error = GetLastError();
		}
	}

	// Check if pipe was broken
	if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED)
	{
		std::cout << "Pipe disconnected" << std::endl;
		mConnected = false;

		// Server: Reset pipe for new connection
		if (mRole == PipeRole::Server)
		{
			DisconnectNamedPipe(mPipe);
		}
	}
	else if (error != ERROR_SUCCESS && error != ERROR_OPERATION_ABORTED)
	{
		std::cerr << "Pipe read failed: " << error << std::endl;
		mConnected = false;
	}

	return 0;
}

void NamedPipeTransport::cleanup()
{
	mConnected = false;

	if (mReadOverlapped.hEvent)
	{
		CloseHandle(mReadOverlapped.hEvent);
		mReadOverlapped.hEvent = nullptr;
	}

	if (mWriteOverlapped.hEvent)
	{
		CloseHandle(mWriteOverlapped.hEvent);
		mWriteOverlapped.hEvent = nullptr;
	}

	if (mPipe != INVALID_HANDLE_VALUE)
	{
		if (mRole == PipeRole::Server)
		{
			DisconnectNamedPipe(mPipe);
		}

		CloseHandle(mPipe);
		mPipe = INVALID_HANDLE_VALUE;
	}
}

bool NamedPipeTransport::isPermanentError(DWORD error) const
{
	// These are errors that won't resolve by retrying
	switch (error)
	{
		case ERROR_ACCESS_DENIED:
		case ERROR_INVALID_PARAMETER:
		case ERROR_INVALID_HANDLE:
		case ERROR_NOT_SUPPORTED:
			return true;

		// These are transient - server not ready, pipe busy, etc.
		case ERROR_PIPE_BUSY:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PIPE_NOT_CONNECTED:
		case ERROR_BROKEN_PIPE:
			return false;

		default:
			// Unknown error - treat as transient and retry
			return false;
	}
}
