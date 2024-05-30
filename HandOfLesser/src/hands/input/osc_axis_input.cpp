#include "osc_axis_input.h"
#include <cstdio>

namespace HOL
{
	void OscAxisInput::submit(Eigen::Vector2f inputData)
	{
		VRChat::VRChatInput::Current->submitFloat(this->mXInput, inputData.x());
		VRChat::VRChatInput::Current->submitFloat(this->mYInput, inputData.y());
	}
} // namespace HOL