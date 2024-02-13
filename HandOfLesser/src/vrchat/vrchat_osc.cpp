#include "vrchat_osc.h"
#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include <oscpp/client.hpp>

namespace HOL::VRChat
{
	float VRChatOSC::HUMAN_RIG_RANGE[SINGLE_HAND_JOINT_COUNT];
	float VRChatOSC::HUMAN_RIG_CENTER[SINGLE_HAND_JOINT_COUNT];

	std::string VRChatOSC::OSC_PARAMETER_NAMES_FULL[VRChat::BOTH_HAND_JOINT_COUNT];
	std::string VRChatOSC::OSC_PARAMETER_NAMES_ALTERNATING[VRChat::SINGLE_HAND_JOINT_COUNT];
	std::string VRChatOSC::OSC_PARAMETER_NAMES_PACKED[VRChat::SINGLE_HAND_JOINT_COUNT];

	VRChatOSC::VRChatOSC()
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
		// The paramNames must match what we're using on the unity side, which is mostly based on
		// what the humanoid rig uses, but we lower-case everything but the HOL/ prefix.

		std::string sideParamNames[2];
		std::string fingerParamName[5];
		std::string bendParamName[4];

		std::string prefix = VRChat::OSC_PREFIX + VRChat::NAMESPACE_PREFIX;

		sideParamNames[HandSide::LeftHand] = "lefthand_";
		sideParamNames[HandSide::RightHand] = "righthand_";

		fingerParamName[FingerType::FingerIndex] = "index_";
		fingerParamName[FingerType::FingerMiddle] = "middle_";
		fingerParamName[FingerType::FingerRing] = "ring_";
		fingerParamName[FingerType::FingerLittle] = "little_";
		fingerParamName[FingerType::FingerThumb] = "thumb_";

		bendParamName[FingerBendType::CurlFirst] = "1_curl";
		bendParamName[FingerBendType::CurlSecond] = "2_curl";
		bendParamName[FingerBendType::CurlThird] = "3_curl";
		bendParamName[FingerBendType::Splay] = "splay";

		// Full
		for (int side = 0; side < HandSide_MAX; side++)
		{
			for (int i = 0; i < FingerType::FingerType_MAX; i++)
			{
				for (int j = 0; j < FingerBendType::FingerBendType_MAX; j++)
				{
					// Note that this includes the side as well
					int index = getParameterIndex((HandSide)side, (FingerType)i, (FingerBendType)j);
					VRChatOSC::OSC_PARAMETER_NAMES_FULL[index]
						= prefix + VRChat::OSC_FULL_PREFIX + sideParamNames[side]
						  + fingerParamName[i] + bendParamName[j];
				}
			}
		}

		// Alternating and packed
		for (int i = 0; i < FingerType::FingerType_MAX; i++)
		{
			for (int j = 0; j < FingerBendType::FingerBendType_MAX; j++)
			{
				// Note we are not including a side
				int index = getParameterIndex((FingerType)i, (FingerBendType)j);
				VRChatOSC::OSC_PARAMETER_NAMES_ALTERNATING[index]
					= prefix + VRChat::OSC_ALTERNATING_PREFIX + fingerParamName[i]
					  + bendParamName[j];
				VRChatOSC::OSC_PARAMETER_NAMES_PACKED[index]
					= prefix + VRChat::OSC_PACKED_PREFIX + fingerParamName[i] + bendParamName[j];
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

	int HOL::VRChat::VRChatOSC::getParameterIndex(HOL::HandSide side,
												  HOL::FingerType finger,
												  HOL::FingerBendType joint)
	{
		// See the other getParameterIndex() function.
		// Basically add SINGLE_HAND_JOINT_COUNT if right hand
		return (finger * 4) + joint + (SINGLE_HAND_JOINT_COUNT * side);
	}

	// Individual joint on one hand
	float HOL::VRChat::VRChatOSC::computeParameterValue(float rawValue,
														HOL::HandSide side,
														HOL::FingerType finger,
														HOL::FingerBendType joint)
	{
		int index = getParameterIndex(finger, joint);

		// Hardcode for now, need to match stuff generated in unity
		MotionRange jointRange;

		// Unity humanoid rig curl goes from -1 closed to 1 open.
		// The rotation between these two values is listed in VRChatOSC::HUMAN_RIG_RANGE.
		// We need to rotate a bit more than this in some cases, so we can extend the range.
		// The values used must be mirrored by the animations created in unity.

		float center;
		if (joint == HOL::FingerBendType::Splay)
		{
			// Splay is always inwards towards the center of the hand,
			// so it will be different for left/right hands. -1 in, 1 out.
			// Thumb is -1: downwards ( towards palm ), 1: upwards.
			center = HOL::settings::FingerSplayCenter[finger];
			jointRange = HOL::settings::FingersplayRange[finger];

			// Raw values are provided, so we do the conversion for unity here.
			// I /think/ all we need to do is flip the input
			if (side == HandSide::LeftHand)
			{
				// If so, index and middle flipped.
				// Thumb splay moves in the opposite direction, so flip on right hand.
				switch (finger)
				{
					case HOL::FingerIndex:
					case HOL::FingerMiddle:
					{
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
					case HOL::FingerRing: 
					case HOL::FingerThumb: 
					{
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
				jointRange = HOL::settings::ThumbCurlRange[joint];
			}
			else
			{
				center = HOL::settings::CommonCurlCenter[joint];
				jointRange = HOL::settings::CommonCurlRange[joint];
			}
		}

		// For whatever reason these offsets all want to be negative.
		// Since they don't really mean anything to anyone, invert them
		// so the users doesn't need to config negative values.
		center = -center;

		// Any user inputs, such as settings, will be in degrees rather than radians
		center = HOL::degreesToRadians(center);
		// I guess we can just add the center as an offset and it'll work
		// Makes the math a lot simpler
		rawValue += center; 

		// Stock range being the rotation between -1 and 1
		float halfStockRange = VRChatOSC::HUMAN_RIG_RANGE[index] * 0.5f;

		// if both raw ranges are the same, this will span from -range to +range
		// values are in radians.
		float startRange = halfStockRange * jointRange.start;
		float endRange = halfStockRange * jointRange.end;

		// Ratio of RawValue between rangeStart and rangeEnd
		float outputCurl = (rawValue - startRange) / (endRange - startRange);
		// clamp between 0 and 1
		outputCurl = std::clamp(outputCurl, 0.f, 1.f);

		// scale our 0-1 -> -1 to 1
		return (outputCurl * 2.f) - 1.f;
	}

	float HOL::VRChat::VRChatOSC::encodePacked(float left, float right)
	{
		// This was originally written with 0-1 values in mind, but now we're reusing
		// the value from the non-packed OSC params, so scale from -1/1 to 0/1
		left = (left + 1.f) * 0.5f;
		right = (right + 1.f) * 0.5f;

		// Each joint has 16 steps. There are 256 animations.
		// In the first 16, the left joint is at 0, and the right cycles through its 16 steps.
		// Set the left joint to 1 and repeat for the next 16. Repeat.
		// So the value of the left joint will be 0-1 scaled to 0-15, and multiplied by 16.
		// Right will be 0-1 scaled to 0-15, added to the value we calculated for left.

		// 0-1 to 0-15, multiply by 16 to get the first entry of the group of 16 where left is in
		// the correct position, 0, 16, 32, 48, 64 ... 240
		float leftEncoded = roundf((left * 15.f)) * 16.f;

		// the same, except do not multiply by 16 because this is the index inside the group of 16
		float rightEncoded = roundf((right * 15.f));

		// Adding the two together, we get two separate 0-15 values encoded in a single 0-255 value.
		// Left will be in the same position for values 0 to 15, 16 to 31, and so on.
		float packed = leftEncoded + rightEncoded;

		// 0 to 255, but we need this to be in the range -1 to 1 for vrchat and the blendtrees.
		// vrchat will ultimately transmit the float value as an 8-bit float, so there will only
		// be 256 steps between -1 and 1
		return ((packed / 255.f) * 2.f) - 1.f;
	}

	void HOL::VRChat::VRChatOSC::generateOscOutputPacked()
	{
		for (int i = 0; i < FingerType::FingerType_MAX; i++)
		{
			for (int j = 0; j < FingerBendType_MAX; j++)
			{
				int leftSideIndex
					= getParameterIndex(HandSide::LeftHand, (FingerType)i, (FingerBendType)j);
				int rightSideIndex
					= getParameterIndex(HandSide::RightHand, (FingerType)i, (FingerBendType)j);

				float leftBend = this->mOscOutput[leftSideIndex];
				float rightBend = this->mOscOutput[rightSideIndex];

				HOL::display::FingerTracking[HandSide::LeftHand].humanoidBend[i].bend[j] = leftBend;
				HOL::display::FingerTracking[HandSide::RightHand].humanoidBend[i].bend[j]
					= rightBend;

				int index = getParameterIndex((FingerType)i, (FingerBendType)j);
				float packed = encodePacked(leftBend, rightBend);
				this->mOscOutputPacked[index] = packed;

				// 0-255 values in left hand slot, -1 to +1 values in right hand slot
				// We're recreating the 0-255 values from the -1 to +1 for display purposes
				HOL::display::FingerTracking[HandSide::LeftHand].packedBend[i].bend[j]
					= std::roundf(((packed + 1.f) * 0.5f) * 255.f);
				HOL::display::FingerTracking[HandSide::RightHand].packedBend[i].bend[j] = packed;
			}
		}
	}

	void HOL::VRChat::VRChatOSC::generateOscOutputFull(HOL::HandPose& leftHand,
													   HOL::HandPose& rightHand)
	{
		for (int side = 0; side < HandSide::HandSide_MAX; side++)
		{
			HandPose& hand = (side == HandSide::LeftHand) ? leftHand : rightHand;

			for (int i = 0; i < FingerType::FingerType_MAX; i++)
			{
				FingerBend& finger = hand.fingers[i];

				for (int j = 0; j < FingerBendType_MAX; j++)
				{
					float bend = computeParameterValue(
						finger.bend[j], (HandSide)side, (FingerType)i, (FingerBendType)j);

					HOL::display::FingerTracking[side].humanoidBend[i].bend[j] = bend;

					int index = getParameterIndex((HandSide)side, (FingerType)i, (FingerBendType)j);
					this->mOscOutput[index] = bend;
				}
			}
		}
	}

	// Generates the values for one hand at the time
	void HOL::VRChat::VRChatOSC::generateOscOutput(HOL::HandPose& leftHand,
												   HOL::HandPose& rightHand)
	{
		generateOscOutputFull(leftHand, rightHand);
		generateOscOutputPacked();
	}

	// includes a hand_side param denoting which hand the data is for
	size_t HOL::VRChat::VRChatOSC::generateOscBundleAlternating()
	{
		HOL::HandSide side = this->swapTransmitSide();

		// 0 or 40 depending on left or right side
		// Use to offset i
		int sideIndexOffset = VRChat::SINGLE_HAND_JOINT_COUNT * side;

		// Size param is presumably buffer size
		OSCPP::Client::Packet packet(this->mOscPacketBuffer, OSC_PACKET_BUFFER_SIZE);

		packet.openBundle(0);

		for (int i = 0; i < SINGLE_HAND_JOINT_COUNT; i++)
		{
			packet.openMessage(VRChatOSC::OSC_PARAMETER_NAMES_ALTERNATING[i].c_str(), 1)
				.float32(this->mOscOutput[i + sideIndexOffset])
				.closeMessage();
		}

		packet.openMessage(OSC_ALTERNATING_HAND_SIDE_PARAMETER.c_str(), 1)
			.int32(side)
			.closeMessage();

		packet.closeBundle();

		return packet.size();
	}

	size_t HOL::VRChat::VRChatOSC::generateOscBundlePacked()
	{
		// Size param is presumably buffer size
		OSCPP::Client::Packet packet(this->mOscPacketBuffer, OSC_PACKET_BUFFER_SIZE);

		packet.openBundle(0);

		for (int i = 0; i < SINGLE_HAND_JOINT_COUNT; i++)
		{
			packet.openMessage(VRChatOSC::OSC_PARAMETER_NAMES_PACKED[i].c_str(), 1)
				.float32(this->mOscOutputPacked[i])
				.closeMessage();
		}

		packet.closeBundle();

		return packet.size();
	}

	size_t HOL::VRChat::VRChatOSC::generateOscBundleFull()
	{
		// Size param is presumably buffer size
		OSCPP::Client::Packet packet(this->mOscPacketBuffer, OSC_PACKET_BUFFER_SIZE);

		packet.openBundle(0);

		for (int i = 0; i < BOTH_HAND_JOINT_COUNT; i++)
		{
			packet.openMessage(VRChatOSC::OSC_PARAMETER_NAMES_FULL[i].c_str(), 1)
				.float32(this->mOscOutput[i])
				.closeMessage();
		}

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

} // namespace HOL::VRChat
