#pragma once

#include <nlohmann/json.hpp>
#include <src/settings/settings.h>
#include <src/json/serializer.h>

// json definitions for various structs

namespace HOL
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MotionRange, start, end);

	namespace settings
	{

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FingerBendSettings,
										   commonCurlCenter,
										   thumbCurlCenter,
										   fingerSplayCenter,
										   commonCurlRange,
										   thumbCurlRange,
										   fingersplayRange,
										   ThumbAxisOffset);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SkeletalBendSettings,
										   commonCurlCenter,
										   thumbCurlCenter,
										   fingerSplayCenter,
										   commonCurlRange,
										   thumbCurlRange,
										   fingersplayRange);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GeneralSettings,
										   motionPredictionMS,
										   updateIntervalMS,
										   linearVelocityMultiplier,
										   angularVelocityMultiplier,
										   forceInactive);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandPoseSettings,
										   controllerMode,
										   controllerType,
										   orientationOffset,
										   positionOffset);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VRChatSettings,
										   sendFull,
										   sendAlternating,
										   sendPacked,
										   interlacePacked,
										   packedUpdateInterval,
										   useUnityHumanoidSplay);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InputSettings, sendOscInput);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SkeletalInput,
										   jointLengthMultiplier,
										   sendSkeletalInput,
										   augmentHookedControllers,
										   positionOffset,
										   orientationOffset);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SteamVRSettings,
										   sendSteamVRControllerPosition,
										   sendSteamVRInput,
										   blockControllerInputWhileHandTracking,
										   steamPoseTimeOffset,
										   forceInactive,
										   jitterLastPoseOnTrackingLoss);

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandOfLesserSettings,
										   general,
										   fingerBend,
										   skeletalBend,
										   handPose,
										   vrchat,
										   input,
										   steamvr,
										   skeletal);
	} // namespace settings

} // namespace HOL
