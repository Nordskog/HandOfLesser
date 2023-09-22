#include "transport.h"

using namespace HOL;

#include <iostream>

void Transport::init(int listenPort)
{
	this->mTransport.init(listenPort);
}

void Transport::send( int port, char* buffer, size_t size )
{
	sockaddr_in addr = UdpTransport::getAddress(port);
	this->mTransport.sendPacket(&addr, buffer, size);
}

size_t Transport::receive()
{
	return this->mTransport.receivePacket(this->mReceiveBuffer, 8192);

}

char* Transport::getReceiveBuffer()
{
	return this->mReceiveBuffer;
}
