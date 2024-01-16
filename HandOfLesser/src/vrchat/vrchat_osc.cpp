#include "vrchat_osc.h"
#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include <oscpp/client.hpp>

float HOL::VRChat::VRChatOSC::HUMAN_RIG_RANGE[PARAMETER_COUNT];
float HOL::VRChat::VRChatOSC::HUMAN_RIG_CENTER[PARAMETER_COUNT];

std::string HOL::VRChat::VRChatOSC::OSC_PARAMETER_NAMES[PARAMETER_COUNT];

HOL::VRChat::VRChatOSC::VRChatOSC()
{
	this->mNextNextTransmitSide = HandSide::LeftHand;
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

	// clang-format on
}

void HOL::VRChat::VRChatOSC::initParameterNames()
{
	// Initialize all the osc parmater names so we don't have to generate them for every packet

	std::string fingerParamName[5];
	std::string bendParamName[4];

	std::string prefix = std::string("/avatar/parameters/");

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
			VRChatOSC::OSC_PARAMETER_NAMES[index] = prefix + fingerParamName[i] + bendParamName[j];
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

	VRChatOSC::HUMAN_RIG_RANGE[index] = HOL::degreesToRadians(first);
	VRChatOSC::HUMAN_RIG_RANGE[index + 1] = HOL::degreesToRadians(second);
	VRChatOSC::HUMAN_RIG_RANGE[index + 2] = HOL::degreesToRadians(third);
	VRChatOSC::HUMAN_RIG_RANGE[index + 3] = HOL::degreesToRadians(splay);
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
													HOL::HandSide side,
													HOL::FingerType finger,
													HOL::FingerBendType joint)
{
	int index = getParameterIndex(finger, joint);

	// Hardcode for now, need to match stuff generated in unity
	float rawRangeStart = 0; // Must be <= 0
	float rawRangeEnd = 1;	 // Must be >= 0, but realistically >= 1

	// Unity humanoid rig curl goes from -1 closed to 1 open.
	// From the sake of simplicity, everything below is from 0 open to 1 closed.
	// At the very end of the function this is flipped to make unity happy.
	// Technically we could keep using 0-1, but generating the animations and blendtrees
	// becomes a lot easier on the unity side if we just adhere to their standards.

	// What the center of this range is we're just going to have to eyeball.
	// This will likely also need some adjustment depending on the avatar.
	// For curls this would be about 45 degrees, splays close to 0,
	// but the finger can bend backwawrds a bit too
	float center;
	if (joint == HOL::FingerBendType::Splay)
	{
		// Splay is always inwards towards the center of the hand,
		// so it will be different for left/right hands. 0 in, 1 out.
		// Thumb is 0: downwards ( towards palm ), 1: upwards.
		center = HOL::settings::FingerSplayCenter[finger];

		// Raw values are provided, so we do the conversion for unity here.
		// I /think/ all we need to do is flip the input, but...
		// These are probably going to be wrong for now, but just flip them if they are.
		// What we know for sure now is that all the source values will be ortation in the same
		// direction, probably left
		if (side == HandSide::LeftHand)
		{
			// If so, index and middle flipped. Probably thumb too
			switch (finger)
			{
				case HOL::FingerIndex:
				case HOL::FingerMiddle:
				case HOL::FingerThumb: {
					rawValue *= -1.f;
					break;
				}
				default:
					break;
			}
		}
		else
		{
			// And opposite for right hand
			switch (finger)
			{
				case HOL::FingerLittle:
				case HOL::FingerRing: {
					rawValue *= -1.f;
					break;
				}
				default:
					break;
			}
		}
	}
	else
	{
		if (finger == FingerType::FingerThumb)
		{
			center = HOL::settings::ThumbCurlCenter[joint];
		}
		else
		{
			center = HOL::settings::CommonCurlCenter[joint];
		}
	}

	// Any user inputs, such as settings, will be in degrees rather than radians
	center = HOL::degreesToRadians(center);

	float range = VRChatOSC::HUMAN_RIG_RANGE[index];

	float halfRange = VRChatOSC::HUMAN_RIG_RANGE[index] * 0.5f;

	// rangeStart is where the unity curl value should be 0, and rangeEnd 1.
	// Adjust rawRangeStart and end to extend range of joint, but animations
	// in unity must be generated to match this value.
	// human rig range/center assume an unmodified unity humanoid rig, and should not be
	// modified.
	float rangeStart
		= (center - halfRange) + (rawRangeStart * range); // raw curl equivalent to unity 0
	float rangeEnd
		= (center + halfRange) + ((1.f - rawRangeEnd) * range); // raw curl equivalent to unity 1

	// Ratio of RawValue between rangeStart and rangeEnd
	float outputCurl = (rawValue - rangeStart) / (rangeEnd - rangeStart);
	// clamp between 0 and 1
	outputCurl = std::clamp(outputCurl, 0.f, 1.f);

	// Flip this so it matches unity's inverted open-closed values 
	if (joint != HOL::FingerBendType::Splay)
	{
		outputCurl = 1.f - outputCurl;
	}

	// scale our 0-1 -> -1 to 1, as this is the range OSC and unity's rig support.
	return (outputCurl * 2.f) - 1.f;
}

// Generates the values for one hand at the time
void HOL::VRChat::VRChatOSC::generateOscOutput(HOL::HandPose& hand, HOL::HandSide side)
{
	for (int i = 0; i < FingerType::FingerType_MAX; i++)
	{
		FingerBend finger = hand.fingers[i];

		for (int j = 0; j < FingerBendType_MAX; j++)
		{
			float bend
				= computeParameterValue(finger.bend[j], side, (FingerType)i, (FingerBendType)j);

			
			if (j == FingerBendType::Splay && i == FingerType::FingerLittle)
			{
				// Unity's humanoid rig does not model splaying properly.
				// When your first is close, your fingers roll outwards to point the fingers
				// inwards. This is the same rotation as when when full splayed outwards when
				// the hand is open. Unity instead treats it as a left-right rotation in the
				// direction the finger is pointing, which results in the fingers pointing
				// outwards instead of in, in a very broken manner. Ultimately we will switch to
				// using bone animations instead, but sticking with humanoid for testing
				// purposes. Splay should blend to -1 as it approached a curl of 90 degrees, to
				// make humanoid look vaugely reasonable.
				float firstJointCurlAmount = computeParameterValue(finger.bend[FingerBendType::CurlFirst],
											side,
											(FingerType)i,
											FingerBendType::CurlFirst);

				// Fip this so 1 is closed, -1 open
				float offset = 0.25f;  // start from -.25
				firstJointCurlAmount = -firstJointCurlAmount;

				float ratio = firstJointCurlAmount + offset / 1.f + offset;
				if (ratio < 0)
					ratio = 0;

				bend = (bend * (1.f - ratio)) + (ratio * -1.f);	
			}
			
			HOL::display::FingerTracking[side].humanoidBend[i].bend[j] = bend;

			int index = getParameterIndex((FingerType)i, (FingerBendType)j);
			this->mOscOutput[index] = bend;
		}
	}
}

// includes a hand_side param denoting which hand the data is for
size_t HOL::VRChat::VRChatOSC::generateOscBundle(HOL::HandSide side)
{
	// Size param is presumably buffer size
	OSCPP::Client::Packet packet(this->mOscPacketBuffer, PARAMETER_COUNT * 128);

	packet.openBundle(0);

	for (int i = 0; i < PARAMETER_COUNT; i++)
	{
		packet.openMessage(VRChatOSC::OSC_PARAMETER_NAMES[i].c_str(), 1)
			.float32(this->mOscOutput[i])
			.closeMessage();
	}

	packet.openMessage("/avatar/parameters/hand_side", 1).int32(side).closeMessage();

	packet.closeBundle();

	return packet.size();
}

char* HOL::VRChat::VRChatOSC::getPacketBuffer()
{
	return this->mOscPacketBuffer;
}

HOL::HandSide HOL::VRChat::VRChatOSC::swapTransmitSide()
{
	// Swap side, return new side.
	this->mNextNextTransmitSide = this->mNextNextTransmitSide == HandSide::LeftHand
									  ? HandSide::RightHand
									  : HandSide::LeftHand;

	return this->mNextNextTransmitSide;
}
