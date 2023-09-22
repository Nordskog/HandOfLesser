#pragma once

namespace HOL
{
	bool ensureWSAStartup();
	void printWSAError(const char* message);
	bool initSocketWSA();
}



