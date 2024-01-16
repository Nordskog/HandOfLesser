#pragma once

#include <HandOfLesserCommon.h>
#include "src/hands/hand_pose.h"

namespace HOL::VRChat
{
	static const int PARAMETER_COUNT
		= FingerType::FingerType_MAX * FingerBendType::FingerBendType_MAX;

	class VRChatOSC
	{

	public:
		VRChatOSC();
		static int getParameterIndex(HOL::FingerType finger, HOL::FingerBendType joint);
		static float computeParameterValue(float rawValue,
										   HOL::HandSide side,
										   HOL::FingerType finger,
										   HOL::FingerBendType joint);

		void generateOscOutput(HOL::HandPose& hand, HOL::HandSide side);

		// these guys need to be easily configurable
		static float HUMAN_RIG_RANGE[PARAMETER_COUNT];
		static float HUMAN_RIG_CENTER[PARAMETER_COUNT];

		size_t generateOscBundle(HOL::HandSide side);

		char* getPacketBuffer();

		HOL::HandSide swapTransmitSide();

	private:
		static void initParameters();
		static void initParameterNames();
		static void setHumanRigRange(HOL::FingerType finger,
									 float first,
									 float second,
									 float third,
									 float splay);
		HOL::HandSide mNextNextTransmitSide;
		static std::string OSC_PARAMETER_NAMES[PARAMETER_COUNT];
		float mOscOutput[PARAMETER_COUNT];
		char mOscPacketBuffer[PARAMETER_COUNT * 128]; // 2560 Should be plenty
	};

} // namespace HOL::VRChat
