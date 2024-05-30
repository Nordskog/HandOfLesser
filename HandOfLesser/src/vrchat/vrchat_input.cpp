#pragma once

#include "src/hands/input/base_input.h"
#include <string>
#include "vrchat_input.h"

namespace HOL::VRChat
{
	VRChatInput* VRChatInput::Current = nullptr;

	VRChatInput::VRChatInput()
	{
		VRChatInput::Current = this;
	}

	std::tuple<char*, size_t> HOL::VRChat::VRChatInput::finalizeInputBundle()
	{
		if (this->mPacketOpen)
		{
			this->mPacket.closeBundle();
			this->mPacketOpen = false;
		}

		return std::make_tuple(this->mOscPacketBuffer, this->mPacket.size());
	}
	void VRChatInput::submitFloat(std::string& inputName, float value)
	{
		if (!this->mPacketOpen)
		{
			this->mPacket
				= OSCPP::Client::Packet(this->mOscPacketBuffer, OSC_INPUT_PACKET_BUFFER_SIZE);
			this->mPacket.openBundle(0);
			this->mPacketOpen = true;
		}

		this->mPacket.openMessage((VRChat::OSC_INPUT_PREFIX + inputName).c_str(), 1)
			.float32(value)
			.closeMessage();
	}
} // namespace HOL::VRChat