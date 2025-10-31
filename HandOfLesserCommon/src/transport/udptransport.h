#pragma once

#include <winsock2.h>
#include "itransport.h"

namespace HOL
{
	class UdpTransport : public ITransport
	{
	public:
		UdpTransport() = default;
		~UdpTransport();

		// Initialize: sendPort = destination port for sending (required)
		//             listenPort = port to bind for receiving (0 = send-only)
		//             sendAddress = destination IP (default: loopback)
		bool init(int sendPort, int listenPort = 0, const char* sendAddress = "127.0.0.1");
		void shutdown() override;
		size_t send(const char* buffer, size_t size) override;
		size_t receive(char* buffer, size_t maxSize) override;
		bool isConnected() const override
		{
			return mInitialized;
		}

	private:
		SOCKET mSocket = INVALID_SOCKET;
		int mListenPort;
		int mSendPort;
		sockaddr_in mSendAddr = {};
		fd_set mFdSet;
		timeval mTimeout;
		bool mInitialized = false;
	};
} // namespace HOL
