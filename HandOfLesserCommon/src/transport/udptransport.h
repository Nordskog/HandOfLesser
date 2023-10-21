#pragma once

#include <winsock2.h>

namespace HOL
{
class UdpTransport
{
public:
	bool init(int port);
	size_t receivePacket(char* buffer, size_t maxlength);
	size_t sendPacket(sockaddr_in* to, char* buffer, size_t length);
	static sockaddr_in getAddress(int port);

private:
	SOCKET mSocket;
};
} // namespace HOL
