#pragma once

#include "udptransport.h"

namespace HOL
{
class Transport
{
public:
	void init(int listenPort);
	void send(int port, char* buffer, size_t size);
	size_t receive();
	char* getReceiveBuffer();

private:
	UdpTransport mTransport;
	char mReceiveBuffer[8192];
};

} // namespace HOL
