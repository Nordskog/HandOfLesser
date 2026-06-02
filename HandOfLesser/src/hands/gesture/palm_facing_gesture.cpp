#include "palm_facing_gesture.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace HOL::Gesture::PalmFacingGesture
{
	float Gesture::evaluateInternal(GestureData data)
	{
		if (!data.HeadPoseValid || this->parameters.side < HOL::LeftHand
			|| this->parameters.side >= HOL::HandSide_MAX)
		{
			return 0.0f;
		}

		const HOL::HandPose* handPose = data.handPose[this->parameters.side];
		if (handPose == nullptr || !handPose->poseValid)
		{
			return 0.0f;
		}

		Eigen::Vector3f toHead = data.HeadPose.position - handPose->palmLocation.position;
		float headDistanceSquared = toHead.squaredNorm();
		if (headDistanceSquared <= 0.000001f)
		{
			return 0.0f;
		}

		Eigen::Vector3f palmFacing = handPose->palmLocation.orientation * -Eigen::Vector3f::UnitY();
		if (palmFacing.squaredNorm() <= 0.000001f)
		{
			return 0.0f;
		}

		palmFacing.normalize();
		toHead.normalize();

		float clampedFovDegrees = std::clamp(this->parameters.fovDegrees, 1.0f, 179.0f);
		float activationAngleRadians
			= clampedFovDegrees * 0.5f * (std::numbers::pi_v<float> / 180.0f);
		float actualDot = std::clamp(palmFacing.dot(toHead), -1.0f, 1.0f);
		float actualAngleRadians = std::acos(actualDot);

		if (actualAngleRadians <= activationAngleRadians)
		{
			return 1.0f;
		}

		float fadeRangeRadians = std::numbers::pi_v<float> - activationAngleRadians;
		if (fadeRangeRadians <= 0.000001f)
		{
			return 0.0f;
		}

		float normalized
			= 1.0f - ((actualAngleRadians - activationAngleRadians) / fadeRangeRadians);
		return std::clamp(normalized, 0.0f, 1.0f);
	}
} // namespace HOL::Gesture::PalmFacingGesture
