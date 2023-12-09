#include "vrchat_osc.h"

float HOL::VRChat::VRChatOSC::HUMAN_RIG_RANGE[PARAMETER_COUNT];
float HOL::VRChat::VRChatOSC::HUMAN_RIG_CENTER[PARAMETER_COUNT];
std::tuple<float, float> HOL::VRChat::VRChatOSC::RAW_RANGE[PARAMETER_COUNT];

std::string HOL::VRChat::VRChatOSC::OSC_PARAMETER_NAMES[PARAMETER_COUNT];
float HOL::VRChat::VRChatOSC::OSC_OUTPUT[PARAMETER_COUNT];

void HOL::VRChat::VRChatOSC::init()
{
	initParameters();
	initParameterNames();
}

void HOL::VRChat::VRChatOSC::initParameters()
{
	// clang-format off
	// 
	// Unity's humanoid rig expects value in the range 0 - 1 
	// This range represents the "normal" range of motion of the given joint.
	// In order to translate our raw rotation into this range, we need:
	// A: The range of motion ( e.g. around 90 degrees for finger curls )
	// B: The orientation that corresponds to the center of this range
	// The former we can pull directly from unity, the latter requires eyeballing and adjustment.

	// These values are pulled from unity and should not need to be modified.
	setHumanRigRange(FingerType::FingerIndex,	100, 90, 90, 40);
	setHumanRigRange(FingerType::FingerMiddle,	100, 90, 90, 15);
	setHumanRigRange(FingerType::FingerRing,	100, 90, 90, 15);
	setHumanRigRange(FingerType::FingerLittle,	100, 90, 90, 40);

	setHumanRigRange(FingerType::FingerThumb,	40, 75, 75, 50);

	// What the center of this range is we're just going to have to eyeball.
	// This will likely also need some adjustment depending on the avatar.
	// For curls this would be about 45 degrees, splays close to 0,
	// but the finger can bend backwawrds a bit too
	setHumanRigCenter(FingerType::FingerIndex,	45, 45, 45, 0);
	setHumanRigCenter(FingerType::FingerMiddle,	45, 45, 45, 0);
	setHumanRigCenter(FingerType::FingerRing,	45, 45, 45, 0);
	setHumanRigCenter(FingerType::FingerLittle,	45, 45, 45, 0);
	
	// Thumb is weird
	setHumanRigCenter(FingerType::FingerThumb,	0, 22, 22, 0);

	// clang-format on
}

void HOL::VRChat::VRChatOSC::initParameterNames()
{
	// Initialize all the osc parmater names so we don't have to generate them for every packet

	std::string fingerParamName[5];
	std::string bendParamName[4];

	fingerParamName[FingerType::FingerIndex] = "index_";
	fingerParamName[FingerType::FingerMiddle] = "middle_";
	fingerParamName[FingerType::FingerRing] = "ring_";
	fingerParamName[FingerType::FingerLittle] = "little_";
	fingerParamName[FingerType::FingerThumb] = "thumb_";

	bendParamName[FingerBendType::CurlFirst] = "1_curl";
	bendParamName[FingerBendType::CurlSecond] = "2_curl";
	bendParamName[FingerBendType::CurlThird] = "3_curl";
	bendParamName[FingerBendType::Splay] = "splay";

	for (int i = 0; i < FingerType::FingerType_MAX; i++)
	{
		for (int j = 0; j < FingerBendType::FingerBendType_MAX; j++)
		{
			int index = getParameterIndex((FingerType)i, (FingerBendType)j);
			VRChatOSC::OSC_PARAMETER_NAMES[index] = fingerParamName[i] + bendParamName[j];
		}
	}
}

void HOL::VRChat::VRChatOSC::setHumanRigRange(HOL::FingerType finger,
											  float first,
											  float second,
											  float third,
											  float splay)
{
	// Indieces are in sequence
	int index = VRChatOSC::getParameterIndex(finger, HOL::FingerBendType::CurlFirst);

	VRChatOSC::HUMAN_RIG_RANGE[index] = first;
	VRChatOSC::HUMAN_RIG_RANGE[index + 1] = second;
	VRChatOSC::HUMAN_RIG_RANGE[index + 2] = third;
	VRChatOSC::HUMAN_RIG_RANGE[index + 3] = splay;
}

void HOL::VRChat::VRChatOSC::setHumanRigCenter(HOL::FingerType finger,
											   float first,
											   float second,
											   float third,
											   float splay)
{
	// Indieces are in sequence
	int index = VRChatOSC::getParameterIndex(finger, HOL::FingerBendType::CurlFirst);

	VRChatOSC::HUMAN_RIG_CENTER[index] = first;
	VRChatOSC::HUMAN_RIG_CENTER[index + 1] = second;
	VRChatOSC::HUMAN_RIG_CENTER[index + 2] = third;
	VRChatOSC::HUMAN_RIG_CENTER[index + 3] = splay;
}

int HOL::VRChat::VRChatOSC::getParameterIndex(HOL::FingerType finger, HOL::FingerBendType joint)
{
	// There are 2 total parameters, 3 splay and 1 curl for each finger
	// joint denotes the curl as 1,2,3, with 4 being used for splay
	// All these values are internal and arbitrary
	return (finger * 4) + joint;
}

void HOL::VRChat::VRChatOSC::computeParameterValue(float rawValue,
												   HOL::HandSide,
												   HOL::FingerType finger,
												   HOL::FingerBendType joint)
{
}
