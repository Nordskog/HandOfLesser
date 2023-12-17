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

// Individual joint on one hand
float HOL::VRChat::VRChatOSC::computeParameterValue(float rawValue,
													HOL::HandSide,
													HOL::FingerType finger,
													HOL::FingerBendType joint)
{
	int index = getParameterIndex(finger, joint);

	// Hardcode for now, need to match stuff generated in unity
	float rawRangeStart = 0; // Must be <= 0
	float rawRangeEnd = 1;	 // Must be >= 0, but realistically >= 1

	if (joint == HOL::FingerBendType::Splay)
	{
		// Splay is always inwards towards the center of the hand,
		// so it will be different for left/right hands. Also there's the thumb.
	}
	else
	{
		float range = VRChatOSC::HUMAN_RIG_RANGE[index];
		float center = VRChatOSC::HUMAN_RIG_CENTER[index];
		float halfRange = VRChatOSC::HUMAN_RIG_RANGE[index] * 0.5f;

		// rangeStart is where the unity curl value should be 0, and rangeEnd 1.
		// Adjust rawRangeStart and end to extend range of joint, but animations
		// in unity must be generated to match this value.
		// human rig range/center assume an unmodified unity humanoid rig, and should not be
		// modified.
		float rangeStart = (center - halfRange) + (rawRangeStart * range); // raw curl equivalent to unity 0
		float rangeEnd = (center + halfRange) + (rawRangeEnd * range);	   // raw curl equivalent to unity 1

		// Ratio of RawValue between rangeStart and rangeEnd
		float outputCurl = (rawValue - rangeStart) / (rangeEnd - rangeStart);
		// clamp between 0 and 1
		return std::clamp(outputCurl, 0.f, 1.f);
	}
}

float HOL::VRChat::VRChatOSC::encodePacked(float left, float right)
{
	// Each joint has 16 steps. There are 256 animations.
	// In the first 16, the left joint is at 0, and the right cycles through its 16 steps.
	// Set the left joint to 1 and repeat for the next 16. Repeat.
	// So the value of the left joint will be 0-1 scaled to 0-15, and multiplied by 16.
	// Right will be 0-1 scaled to 0-15, added to the value we calculated for left.

	// 0-1 to 0-15, multiply by 16 to get the first entry of the group of 16 where left is in the
	// correct position
	float leftEncoded = roundf((left * 15.f)) * 16.f;

	// the same, except do not multiply by 16 because this is the index inside the group of 16
	float rightEncoded = roundf((right * 15.f));

	// 0 to 255, but we need this to be in the range -1 to 1 for vrchat and the blendtrees.
	// vrchat will ultimately transmit the float value as an 8-bit float, so there will only
	// be 256 steps between -1 and 1
	float packed = leftEncoded + rightEncoded;

	return ((packed / 255.f) * 2.f) - 1.f;
}

void HOL::VRChat::VRChatOSC::generateOscOutput(HOL::HandPose* leftHand, HOL::HandPose* rightHand)
{
	for (int i = 0; i < FingerType::FingerType_MAX; i++)
	{
		FingerBend* leftFinger = &leftHand->fingers[i];
		FingerBend* rightFinger = &rightHand->fingers[i];

		for (int j = 0; j < FingerBendType_MAX; j++)
		{
			float leftHand = computeParameterValue(
				leftFinger->bend[j], HOL::LeftHand, (FingerType)i, (FingerBendType)j);
			float rightHand = computeParameterValue(
				rightFinger->bend[j], HOL::RightHand, (FingerType)i, (FingerBendType)j);

			int index = getParameterIndex((FingerType)i, (FingerBendType)j);
			OSC_OUTPUT[index] = encodePacked(leftHand, rightHand);
		}
	}
}
