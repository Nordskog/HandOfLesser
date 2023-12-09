#pragma once

#include <HandOfLesserCommon.h>

namespace HOL::VRChat
{
	static const int PARAMETER_COUNT
		= FingerType::FingerType_MAX * FingerBendType::FingerBendType_MAX;

	class VRChatOSC
	{

	public:
		static void init();
		static int getParameterIndex(HOL::FingerType finger, HOL::FingerBendType joint);
		static void computeParameterValue(float rawValue,
										  HOL::HandSide,
										  HOL::FingerType finger,
										  HOL::FingerBendType joint);

		static void generateOscOutput();

		// these guys need to be easily configurable
		static float HUMAN_RIG_RANGE[PARAMETER_COUNT];
		static float HUMAN_RIG_CENTER[PARAMETER_COUNT];
		static std::tuple<float, float> RAW_RANGE[PARAMETER_COUNT];

	private:
		static void initParameters();
		static void initParameterNames();
		static void setHumanRigRange(HOL::FingerType finger,
									 float first,
									 float second,
									 float third,
									 float splay);
		static void setHumanRigCenter(HOL::FingerType finger,
									  float first,
									  float second,
									  float third,
									  float splay);
		static std::string OSC_PARAMETER_NAMES[PARAMETER_COUNT];
		static float OSC_OUTPUT[PARAMETER_COUNT];
	};

} // namespace HOL::VRChat
