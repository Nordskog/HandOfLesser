#include "nativetransport.h"

void HOL::NativeTransport::init(int listenPort)
{
	this->mTransport.init(listenPort);
}

void HOL::NativeTransport::send(int port, char* buffer, size_t size)
{
	this->mTransport.send(port, buffer, size);
}

// Pointer valid until next receive() call
HOL::NativePacket* HOL::NativeTransport::receive()
{
	size_t length = this->mTransport.receive();
	if (length > sizeof(HOL::NativePacket))
	{
		// Do proper checks at some point
		return (HOL::NativePacket*)(this->mTransport.getReceiveBuffer());
	}
	else
	{
		return nullptr;
	}
}
