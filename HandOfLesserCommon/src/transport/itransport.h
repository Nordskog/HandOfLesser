#pragma once

#include <cstddef>
#include "src/packet/nativepacket.h"

namespace HOL
{

	/**
	 * Abstract base interface for all transport implementations.
	 * Provides common send/receive/lifecycle methods.
	 */
	class ITransport
	{
	public:
		virtual ~ITransport() = default;

		// Note: init() is NOT part of the interface since different transports
		// require different configuration parameters. Call init() directly on
		// the concrete transport type.

		// Shutdown and cleanup resources
		virtual void shutdown() = 0;

		// Explicit send - returns bytes sent (0 on failure)
		virtual size_t send(const char* buffer, size_t size) = 0;

		// Template send for type safety with packet structures
		template <typename T> size_t send(const T& packet)
		{
			return send((const char*)&packet, sizeof(T));
		}

		// Receive raw data into buffer - returns bytes received (0 on timeout/error)
		// Implementations should support a reasonable timeout (e.g., 1 second)
		virtual size_t receive(char* buffer, size_t maxSize) = 0;

		// Receive and validate NativePacket (null if invalid/timeout)
		// Pointer valid until next receive call
		// Default implementation uses mReceiveBuffer
		virtual NativePacket* receivePacket()
		{
			size_t length = receive(mReceiveBuffer, sizeof(mReceiveBuffer));
			if (length > sizeof(NativePacket))
			{
				return (NativePacket*)mReceiveBuffer;
			}
			return nullptr;
		}

		// Check if transport is connected/ready
		// For connectionless protocols (UDP), always returns true after init
		virtual bool isConnected() const = 0;

	protected:
		// Buffer for receivePacket() implementation
		char mReceiveBuffer[8192] = {};
	};

} // namespace HOL
