#pragma once

#include "transportutil.h"
#include <iostream>
#include <winsock2.h>

namespace HOL
{
static bool WsaStartup = false;

void printWSAError(const char* message)
{
	int error = WSAGetLastError();
	std::cerr << message << ", WSA error code: " << error << std::endl;
}

bool ensureWSAStartup()
{
	return initSocketWSA();
}

bool initSocketWSA()
{
	if (WsaStartup)
	{
		return true;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printWSAError("WSAStartup failed");
		return false;
	}

	WsaStartup = true;

	return true;
}

} // namespace HOL
