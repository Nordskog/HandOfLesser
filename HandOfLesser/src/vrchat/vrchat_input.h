#pragma once

#include "src/hands/input/base_input.h"
#include <string>
#include <oscpp/client.hpp>

namespace HOL::VRChat
{
	static const int OSC_INPUT_PACKET_BUFFER_SIZE = 2560; // Big number

	class VRChatInput
	{
	public:
		VRChatInput();
		std::tuple<char*, size_t> finalizeInputBundle();
		void submitFloat(std::string& inputName, float value);
		static VRChatInput* Current;

	private:
		char mOscPacketBuffer[OSC_INPUT_PACKET_BUFFER_SIZE]; // Big number
		OSCPP::Client::Packet mPacket;
		bool mPacketOpen = false; // need to init packet
	};

} // namespace HOL::VRChat