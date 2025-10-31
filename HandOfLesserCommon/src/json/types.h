#pragma once

#include <nlohmann/json.hpp>
#include <src/settings/settings.h>
#include <src/json/serializer.h>

namespace HOL
{
	inline void to_json(nlohmann::json& j, const MotionRange& range)
	{
		j = nlohmann::json{{"start", range.start}, {"end", range.end}};
	}

	inline void from_json(const nlohmann::json& j, MotionRange& range)
	{
		nlohmann::get_to_if_present(j, "start", range.start);
		nlohmann::get_to_if_present(j, "end", range.end);
	}

	namespace settings
	{
		inline void to_json(nlohmann::json& j, const FingerBendSettings& settings)
		{
			j = {{"commonCurlCenter", settings.commonCurlCenter},
				 {"thumbCurlCenter", settings.thumbCurlCenter},
				 {"fingerSplayCenter", settings.fingerSplayCenter},
				 {"commonCurlRange", settings.commonCurlRange},
				 {"thumbCurlRange", settings.thumbCurlRange},
				 {"fingersplayRange", settings.fingersplayRange},
				 {"ThumbAxisOffset", settings.ThumbAxisOffset}};
		}

		inline void from_json(const nlohmann::json& j, FingerBendSettings& settings)
		{
			nlohmann::get_to_if_present(j, "commonCurlCenter", settings.commonCurlCenter);
			nlohmann::get_to_if_present(j, "thumbCurlCenter", settings.thumbCurlCenter);
			nlohmann::get_to_if_present(j, "fingerSplayCenter", settings.fingerSplayCenter);
			nlohmann::get_to_if_present(j, "commonCurlRange", settings.commonCurlRange);
			nlohmann::get_to_if_present(j, "thumbCurlRange", settings.thumbCurlRange);
			nlohmann::get_to_if_present(j, "fingersplayRange", settings.fingersplayRange);
			nlohmann::get_to_if_present(j, "ThumbAxisOffset", settings.ThumbAxisOffset);
		}

		inline void to_json(nlohmann::json& j, const SkeletalBendSettings& settings)
		{
			j = {{"commonCurlCenter", settings.commonCurlCenter},
				 {"thumbCurlCenter", settings.thumbCurlCenter},
				 {"fingerSplayCenter", settings.fingerSplayCenter},
				 {"commonCurlRange", settings.commonCurlRange},
				 {"thumbCurlRange", settings.thumbCurlRange},
				 {"fingersplayRange", settings.fingersplayRange}};
		}

		inline void from_json(const nlohmann::json& j, SkeletalBendSettings& settings)
		{
			nlohmann::get_to_if_present(j, "commonCurlCenter", settings.commonCurlCenter);
			nlohmann::get_to_if_present(j, "thumbCurlCenter", settings.thumbCurlCenter);
			nlohmann::get_to_if_present(j, "fingerSplayCenter", settings.fingerSplayCenter);
			nlohmann::get_to_if_present(j, "commonCurlRange", settings.commonCurlRange);
			nlohmann::get_to_if_present(j, "thumbCurlRange", settings.thumbCurlRange);
			nlohmann::get_to_if_present(j, "fingersplayRange", settings.fingersplayRange);
		}

		inline void to_json(nlohmann::json& j, const GeneralSettings& settings)
		{
			j = {{"motionPredictionMS", settings.motionPredictionMS},
				 {"updateIntervalMS", settings.updateIntervalMS},
				 {"linearVelocityMultiplier", settings.linearVelocityMultiplier},
				 {"angularVelocityMultiplier", settings.angularVelocityMultiplier},
				 {"forceInactive", settings.forceInactive},
				 {"minTrackedJointsForQuality", settings.minTrackedJointsForQuality}};
		}

		inline void from_json(const nlohmann::json& j, GeneralSettings& settings)
		{
			nlohmann::get_to_if_present(j, "motionPredictionMS", settings.motionPredictionMS);
			nlohmann::get_to_if_present(j, "updateIntervalMS", settings.updateIntervalMS);
			nlohmann::get_to_if_present(
				j, "linearVelocityMultiplier", settings.linearVelocityMultiplier);
			nlohmann::get_to_if_present(
				j, "angularVelocityMultiplier", settings.angularVelocityMultiplier);
			nlohmann::get_to_if_present(j, "forceInactive", settings.forceInactive);
			nlohmann::get_to_if_present(
				j, "minTrackedJointsForQuality", settings.minTrackedJointsForQuality);
		}

		inline void to_json(nlohmann::json& j, const HandPoseSettings& settings)
		{
			j = {{"controllerMode", settings.controllerMode},
				 {"controllerType", settings.controllerType},
				 {"orientationOffset", settings.orientationOffset},
				 {"positionOffset", settings.positionOffset}};
		}

		inline void from_json(const nlohmann::json& j, HandPoseSettings& settings)
		{
			nlohmann::get_to_if_present(j, "controllerMode", settings.controllerMode);
			nlohmann::get_to_if_present(j, "controllerType", settings.controllerType);
			nlohmann::get_to_if_present(j, "orientationOffset", settings.orientationOffset);
			nlohmann::get_to_if_present(j, "positionOffset", settings.positionOffset);
		}

		inline void to_json(nlohmann::json& j, const VRChatSettings& settings)
		{
			j = {{"sendFull", settings.sendFull},
				 {"sendAlternating", settings.sendAlternating},
				 {"sendPacked", settings.sendPacked},
				 {"interlacePacked", settings.interlacePacked},
				 {"packedUpdateInterval", settings.packedUpdateInterval},
				 {"useUnityHumanoidSplay", settings.useUnityHumanoidSplay},
				 {"sendDebugOsc", settings.sendDebugOsc},
				 {"alternateCurlTest", settings.alternateCurlTest},
				 {"curlDebug", settings.curlDebug},
				 {"splayDebug", settings.splayDebug}};
		}

		inline void from_json(const nlohmann::json& j, VRChatSettings& settings)
		{
			nlohmann::get_to_if_present(j, "sendFull", settings.sendFull);
			nlohmann::get_to_if_present(j, "sendAlternating", settings.sendAlternating);
			nlohmann::get_to_if_present(j, "sendPacked", settings.sendPacked);
			nlohmann::get_to_if_present(j, "interlacePacked", settings.interlacePacked);
			nlohmann::get_to_if_present(j, "packedUpdateInterval", settings.packedUpdateInterval);
			nlohmann::get_to_if_present(j, "useUnityHumanoidSplay", settings.useUnityHumanoidSplay);
			nlohmann::get_to_if_present(j, "sendDebugOsc", settings.sendDebugOsc);
			nlohmann::get_to_if_present(j, "alternateCurlTest", settings.alternateCurlTest);
			nlohmann::get_to_if_present(j, "curlDebug", settings.curlDebug);
			nlohmann::get_to_if_present(j, "splayDebug", settings.splayDebug);
		}

		inline void to_json(nlohmann::json& j, const VisualizerSettings& settings)
		{
			j = {{"followLeftHand", settings.followLeftHand},
				 {"followRightHand", settings.followRightHand}};
		}

		inline void from_json(const nlohmann::json& j, VisualizerSettings& settings)
		{
			nlohmann::get_to_if_present(j, "followLeftHand", settings.followLeftHand);
			nlohmann::get_to_if_present(j, "followRightHand", settings.followRightHand);
		}

		inline void to_json(nlohmann::json& j, const DebugSettings& settings)
		{
			j = {{"rotationFix", settings.rotationFix}, {"rotationOut", settings.rotationOut}};
		}

		inline void from_json(const nlohmann::json& j, DebugSettings& settings)
		{
			nlohmann::get_to_if_present(j, "rotationFix", settings.rotationFix);
			nlohmann::get_to_if_present(j, "rotationOut", settings.rotationOut);
		}

		inline void to_json(nlohmann::json& j, const InputSettings& settings)
		{
			j = {{"sendOscInput", settings.sendOscInput}};
		}

		inline void from_json(const nlohmann::json& j, InputSettings& settings)
		{
			nlohmann::get_to_if_present(j, "sendOscInput", settings.sendOscInput);
		}

		inline void to_json(nlohmann::json& j, const SkeletalInput& settings)
		{
			j = {{"jointLengthMultiplier", settings.jointLengthMultiplier},
				 {"sendSkeletalInput", settings.sendSkeletalInput},
				 {"augmentHookedControllers", settings.augmentHookedControllers},
				 {"positionOffset", settings.positionOffset},
				 {"orientationOffset", settings.orientationOffset}};
		}

		inline void from_json(const nlohmann::json& j, SkeletalInput& settings)
		{
			nlohmann::get_to_if_present(j, "jointLengthMultiplier", settings.jointLengthMultiplier);
			nlohmann::get_to_if_present(j, "sendSkeletalInput", settings.sendSkeletalInput);
			nlohmann::get_to_if_present(
				j, "augmentHookedControllers", settings.augmentHookedControllers);
			nlohmann::get_to_if_present(j, "positionOffset", settings.positionOffset);
			nlohmann::get_to_if_present(j, "orientationOffset", settings.orientationOffset);
		}

		inline void to_json(nlohmann::json& j, const SteamVRSettings& settings)
		{
			j = {{"sendSteamVRControllerPosition", settings.sendSteamVRControllerPosition},
				 {"sendSteamVRInput", settings.sendSteamVRInput},
				 {"blockControllerInputWhileHandTracking",
				  settings.blockControllerInputWhileHandTracking},
				 {"steamPoseTimeOffset", settings.steamPoseTimeOffset},
				 {"forceInactive", settings.forceInactive},
				 {"jitterLastPoseOnTrackingLoss", settings.jitterLastPoseOnTrackingLoss}};
		}

		inline void from_json(const nlohmann::json& j, SteamVRSettings& settings)
		{
			nlohmann::get_to_if_present(
				j, "sendSteamVRControllerPosition", settings.sendSteamVRControllerPosition);
			nlohmann::get_to_if_present(j, "sendSteamVRInput", settings.sendSteamVRInput);
			nlohmann::get_to_if_present(j,
										"blockControllerInputWhileHandTracking",
										settings.blockControllerInputWhileHandTracking);
			nlohmann::get_to_if_present(j, "steamPoseTimeOffset", settings.steamPoseTimeOffset);
			nlohmann::get_to_if_present(j, "forceInactive", settings.forceInactive);
			nlohmann::get_to_if_present(
				j, "jitterLastPoseOnTrackingLoss", settings.jitterLastPoseOnTrackingLoss);
		}

		inline void to_json(nlohmann::json& j, const BodyTrackerSettings& settings)
		{
			j = {{"enableBodyTrackers", settings.enableBodyTrackers},
				 {"enableHips", settings.enableHips},
				 {"enableChest", settings.enableChest},
				 {"enableLeftShoulder", settings.enableLeftShoulder},
				 {"enableLeftUpperArm", settings.enableLeftUpperArm},
				 {"enableLeftLowerArm", settings.enableLeftLowerArm},
				 {"enableRightShoulder", settings.enableRightShoulder},
				 {"enableRightUpperArm", settings.enableRightUpperArm},
				 {"enableRightLowerArm", settings.enableRightLowerArm}};
		}

		inline void from_json(const nlohmann::json& j, BodyTrackerSettings& settings)
		{
			nlohmann::get_to_if_present(j, "enableBodyTrackers", settings.enableBodyTrackers);
			nlohmann::get_to_if_present(j, "enableHips", settings.enableHips);
			nlohmann::get_to_if_present(j, "enableChest", settings.enableChest);
			nlohmann::get_to_if_present(j, "enableLeftShoulder", settings.enableLeftShoulder);
			nlohmann::get_to_if_present(j, "enableLeftUpperArm", settings.enableLeftUpperArm);
			nlohmann::get_to_if_present(j, "enableLeftLowerArm", settings.enableLeftLowerArm);
			nlohmann::get_to_if_present(j, "enableRightShoulder", settings.enableRightShoulder);
			nlohmann::get_to_if_present(j, "enableRightUpperArm", settings.enableRightUpperArm);
			nlohmann::get_to_if_present(j, "enableRightLowerArm", settings.enableRightLowerArm);
		}

		inline void to_json(nlohmann::json& j, const HandOfLesserSettings& settings)
		{
			j = {{"general", settings.general},
				 {"fingerBend", settings.fingerBend},
				 {"skeletalBend", settings.skeletalBend},
				 {"handPose", settings.handPose},
				 {"vrchat", settings.vrchat},
				 {"debug", settings.debug},
				 {"steamvr", settings.steamvr},
				 {"visualizer", settings.visualizer},
				 {"input", settings.input},
				 {"skeletal", settings.skeletal},
				 {"bodyTrackers", settings.bodyTrackers}};
		}

		inline void from_json(const nlohmann::json& j, HandOfLesserSettings& settings)
		{
			nlohmann::get_to_if_present(j, "general", settings.general);
			nlohmann::get_to_if_present(j, "fingerBend", settings.fingerBend);
			nlohmann::get_to_if_present(j, "skeletalBend", settings.skeletalBend);
			nlohmann::get_to_if_present(j, "handPose", settings.handPose);
			nlohmann::get_to_if_present(j, "vrchat", settings.vrchat);
			nlohmann::get_to_if_present(j, "debug", settings.debug);
			nlohmann::get_to_if_present(j, "steamvr", settings.steamvr);
			nlohmann::get_to_if_present(j, "visualizer", settings.visualizer);
			nlohmann::get_to_if_present(j, "input", settings.input);
			nlohmann::get_to_if_present(j, "skeletal", settings.skeletal);
			nlohmann::get_to_if_present(j, "bodyTrackers", settings.bodyTrackers);
		}
	} // namespace settings
} // namespace HOL
