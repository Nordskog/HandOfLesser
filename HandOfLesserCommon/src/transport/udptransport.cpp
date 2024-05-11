//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "udptransport.h"

#include <iostream>
#include "transportutil.h"

using namespace HOL;

bool UdpTransport::init(int port)
{
	if (!HOL::ensureWSAStartup())
	{
		return false;
	}

	sockaddr_in addr = getAddress(port);

	this->mSocket = socket(PF_INET, SOCK_DGRAM, 0); // UDP
	if (this->mSocket == INVALID_SOCKET)
	{
		printWSAError("Socket creation failed");
		return false;
	}

	if (bind(this->mSocket, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0)
	{
		printWSAError("Socket bind failed");
		return false;
	}

	// Stuff so we can use select() with a timeout
	this->mTimeout.tv_sec = 1;	 // Timeout in seconds
	this->mTimeout.tv_usec = 0; // Microseconds

	return true;
}

size_t UdpTransport::sendPacket(sockaddr_in* to, char* buffer, size_t length)
{
	size_t sent = sendto(this->mSocket, buffer, length, 0, (sockaddr*)to, sizeof(sockaddr_in));
	if (sent == SOCKET_ERROR)
	{
		printWSAError("Socket send failed");
		return 0;
	}
}

sockaddr_in UdpTransport::getAddress(int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	return addr;
}

size_t UdpTransport::receivePacket(char* buffer, size_t maxlength)
{
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
		return recv(this->mSocket, buffer, maxlength, 0);
	}
}
