#pragma once

#include <HandOfLesserCommon.h>
#include "src/hands/hand_pose.h"
#include <chrono>

namespace HOL::VRChat
{
	static const int SINGLE_HAND_JOINT_COUNT
		= FingerType::FingerType_MAX * FingerBendType::FingerBendType_MAX;
	static const int BOTH_HAND_JOINT_COUNT = SINGLE_HAND_JOINT_COUNT * 2;

	static const int OSC_PACKET_BUFFER_SIZE = SINGLE_HAND_JOINT_COUNT * 2 * 128;

	static const std::string NAMESPACE_PREFIX = "HOL/";
	static const std::string OSC_FULL_PREFIX = "input/";
	static const std::string OSC_ALTERNATING_PREFIX = "alternating/";
	static const std::string OSC_PACKED_PREFIX = "packed/";

	static const std::string OSC_PREFIX = "/avatar/parameters/";

	static const std::string OSC_ALTERNATING_HAND_SIDE_PARAMETER
		= OSC_PREFIX + NAMESPACE_PREFIX + OSC_ALTERNATING_PREFIX + "hand_side";

	static const std::string OSC_PACKED_INTERLACE_BIT
		= OSC_PREFIX + NAMESPACE_PREFIX + OSC_PACKED_PREFIX + "interlace_bit";

	class VRChatOSC
	{

	public:
		VRChatOSC();

		// For packed or alternating
		static int getParameterIndex(HOL::FingerType finger, HOL::FingerBendType joint);
		//For full
		static int getParameterIndex(HOL::HandSide side, HOL::FingerType finger, HOL::FingerBendType joint);
		float computeParameterValue(float rawValue,
										   HOL::HandSide side,
										   HOL::FingerType finger,
										   HOL::FingerBendType joint);

		static std::pair<float, MotionRange>
		getRangeParameters(HOL::HandSide side, HOL::FingerType finger, HOL::FingerBendType joint);

		void generateOscTestOutput();

		float encodePacked(float left, float right);

		float handleInterlacing(float newValue, float oldValue);

		// Should only be sent at 100ms interval
		bool shouldSendPacked();

		// these guys need to be easily configurable
		static float SKELETAL_RIG_RANGE[SINGLE_HAND_JOINT_COUNT];
		static float HUMAN_RIG_RANGE[SINGLE_HAND_JOINT_COUNT];
		static float HUMAN_RIG_CENTER[SINGLE_HAND_JOINT_COUNT];

		void generateOscOutputFull(HOL::HandPose& leftHand, HOL::HandPose& rightHand);

		// requires that full is generated first.
		void generateOscOutputPacked();

		size_t generateOscBundleFull();
		size_t generateOscBundleAlternating();
		size_t generateOscBundlePacked();

		char* getPacketBuffer();

	private:		
		static void initParameters();
		static void initParameterNames();
		static void setHumanRigRange(HOL::FingerType finger,
									 float first,
									 float second,
									 float third,
									 float splay);

		std::chrono::steady_clock::time_point mLastPackedSendTime;
		HOL::HandSide swapTransmitSide();
		HOL::HandSide mNextNextTransmitSide;

		static std::string OSC_PARAMETER_NAMES_FULL[SINGLE_HAND_JOINT_COUNT * 2];
		static std::string OSC_PARAMETER_NAMES_ALTERNATING[SINGLE_HAND_JOINT_COUNT];
		static std::string OSC_PARAMETER_NAMES_PACKED[SINGLE_HAND_JOINT_COUNT];

		float mOscOutput[SINGLE_HAND_JOINT_COUNT * 2];		// Full. This also used for alternating.
		float mOscOutputInterlacedForClamp[SINGLE_HAND_JOINT_COUNT * 2];	//See handleInterlacing()
		float mOscOutputPacked[SINGLE_HAND_JOINT_COUNT];	// Packed, generated from Full.

		char mOscPacketBuffer[OSC_PACKET_BUFFER_SIZE]; // 2560 Should be plenty
	};

} // namespace HOL::VRChat
