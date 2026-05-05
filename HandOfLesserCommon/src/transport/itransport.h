#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include "src/packet/nativepacket.h"

namespace HOL
{
	struct NativePacketView
	{
		NativePacketType packetType = NativePacketType::InvalidPacket;
		const char* payload = nullptr;
		size_t payloadSize = 0;

		explicit operator bool() const
		{
			return payload != nullptr;
		}

		template <typename T> bool copyPayload(T& out) const
		{
			if (payload == nullptr || payloadSize != sizeof(T))
			{
				return false;
			}

			std::memcpy(&out, payload, sizeof(T));
			return true;
		}
	};

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

		// Template send for callers that need to transmit an already-serialized buffer object.
		template <typename T> size_t send(const T& packet)
		{
			return send((const char*)&packet, sizeof(T));
		}

		// Build the exact wire message as [NativePacket header][payload]. Do not send a typed
		// envelope struct directly; padding would become part of the protocol.
		template <NativePacketType Type, typename Payload> size_t sendPayload(const Payload& payload)
		{
			static_assert(sizeof(NativePacket) + sizeof(Payload) <= NativePacketBufferSize);

			NativePacket packet{
				.packetType = Type,
				.payloadSize = static_cast<uint32_t>(sizeof(Payload)),
			};
			std::array<char, sizeof(NativePacket) + sizeof(Payload)> packetBuffer{};
			std::memcpy(packetBuffer.data(), &packet, sizeof(packet));
			std::memcpy(packetBuffer.data() + sizeof(packet), &payload, sizeof(payload));
			return send(packetBuffer.data(), packetBuffer.size());
		}

		// Receive raw data into buffer - returns bytes received (0 on timeout/error)
		// Implementations should support a reasonable timeout (e.g., 1 second)
		virtual size_t receive(char* buffer, size_t maxSize) = 0;

		// Receive and validate a native packet envelope. The payload pointer is valid until the
		// next receive call, and callers should copy it into the expected payload type.
		virtual NativePacketView receivePacket()
		{
			size_t length = receive(mReceiveBuffer, sizeof(mReceiveBuffer));
			if (length < sizeof(NativePacket))
			{
				return {};
			}

			NativePacket packet;
			std::memcpy(&packet, mReceiveBuffer, sizeof(packet));

			constexpr size_t MaxPayloadSize = NativePacketBufferSize - sizeof(NativePacket);
			if (packet.payloadSize > MaxPayloadSize)
			{
				return {};
			}

			const size_t expectedLength = sizeof(NativePacket) + packet.payloadSize;
			if (length != expectedLength)
			{
				return {};
			}

			return NativePacketView{
				.packetType = packet.packetType,
				.payload = mReceiveBuffer + sizeof(NativePacket),
				.payloadSize = packet.payloadSize,
			};
		}

		// Check if transport is connected/ready
		// For connectionless protocols (UDP), always returns true after init
		virtual bool isConnected() const = 0;

	protected:
		// Buffer for receivePacket() implementation
		char mReceiveBuffer[NativePacketBufferSize] = {};
	};

} // namespace HOL
