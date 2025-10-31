//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "udptransport.h"

#include <iostream>
#include <ws2tcpip.h>
#include "transportutil.h"

using namespace HOL;

UdpTransport::~UdpTransport()
{
	shutdown();
}

bool UdpTransport::init(int sendPort, int listenPort, const char* sendAddress)
{
	mListenPort = listenPort;
	mSendPort = sendPort;
	mSendAddr.sin_family = AF_INET;
	mSendAddr.sin_port = htons(sendPort);
	inet_pton(AF_INET, sendAddress, &mSendAddr.sin_addr);

	if (!HOL::ensureWSAStartup())
	{
		return false;
	}

	this->mSocket = socket(PF_INET, SOCK_DGRAM, 0); // UDP
	if (this->mSocket == INVALID_SOCKET)
	{
		printWSAError("Socket creation failed");
		return false;
	}

	// Only bind if we have a listen port (for receiving)
	if (mListenPort > 0)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(mListenPort);
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		if (bind(this->mSocket, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0)
		{
			printWSAError("Socket bind failed");
			closesocket(this->mSocket);
			this->mSocket = INVALID_SOCKET;
			return false;
		}
	}

	// Stuff so we can use select() with a timeout
	this->mTimeout.tv_sec = 1;	// Timeout in seconds
	this->mTimeout.tv_usec = 0; // Microseconds

	mInitialized = true;
	return true;
}

void UdpTransport::shutdown()
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
	mInitialized = false;
}

size_t UdpTransport::send(const char* buffer, size_t size)
{
	if (!mInitialized)
	{
		return 0;
	}

	size_t sent
		= sendto(this->mSocket, buffer, size, 0, (sockaddr*)&mSendAddr, sizeof(sockaddr_in));
	if (sent == SOCKET_ERROR)
	{
		printWSAError("Socket send failed");
		return 0;
	}
	return sent;
}

size_t UdpTransport::receive(char* buffer, size_t maxSize)
{
	if (!mInitialized)
	{
		return 0;
	}

	// Must be done before every call to select()
	// If you only do this once, you'll observe a network-wide lag spike
	// every 5 seconds or so, with a timeout of 1s. Increasing it reduces the symptoms.
	FD_ZERO(&this->mFdSet);
	FD_SET(this->mSocket, &mFdSet);

	size_t res = select(0, &this->mFdSet, nullptr, nullptr, &this->mTimeout);

	if (res == SOCKET_ERROR)
	{
		printWSAError("Socket read failed");
		return 0;
	}
	else if (res == 0)
	{
		return 0;
	}
	else
	{
		return recv(this->mSocket, buffer, maxSize, 0);
	}
}
