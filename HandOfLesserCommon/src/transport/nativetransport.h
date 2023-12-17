#pragma once

#include "transport.h"
#include "src/packet/nativepacket.h"

namespace HOL
{
class NativeTransport
{
public:
	void init(int listenPort);
	void send(int port, char* buffer, size_t size);
	HOL::NativePacket* receive();

private:
	Transport mTransport;
};

} // namespace HOL
