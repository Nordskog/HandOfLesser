#include "hand_drag_action.h"

namespace HOL
{
	HandDragAction::HandDragAction(HandSide side, XrHandJointEXT joint)
	{
		this->mHandSide = side;
		this->mTargetJoint = joint;
	}

	void HandDragAction::onEvaluate(GestureData gestureData, ActionData actionData)
	{
		if (actionData.onDown || actionData.isDown)
		{
			Eigen::Vector3f currentPos = OpenXR::toEigenVector(
				gestureData.joints[this->mHandSide][this->mTargetJoint].pose.position);

			if (actionData.onDown)
			{
				this->mStartPosition = currentPos;
			}

			if (actionData.isDown)
			{
				Eigen::Vector3f diff = currentPos - this->mStartPosition;

				// We don't have access to HMD position in VD, so use hand orientation as reference
				// instead
				Eigen::Quaternionf currentOrientation = OpenXR::toEigenQuaternion(
					gestureData.joints[this->mHandSide][this->mTargetJoint].pose.orientation);

				diff = currentOrientation.inverse() * diff;

				diff.z() = -diff.z();
				diff *= mMultiplier;
				diff.x() = std::clamp(diff.x(), -1.0f, 1.0f);
				diff.z() = std::clamp(diff.z(), -1.0f, 1.0f);

				// TODO deadzone, senstivity, vertical axis
				this->mInputSink->submit(Eigen::Vector2f(diff.x(), diff.z()));
			}
		}
		else if (actionData.onUp)
		{
			this->mInputSink->submit(Eigen::Vector2f::Zero());
		}
	}
} // namespace HOL