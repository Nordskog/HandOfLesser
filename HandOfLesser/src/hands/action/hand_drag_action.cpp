#include "hand_drag_action.h"

namespace HOL
{
	std::shared_ptr<HandDragAction> HandDragAction::setup(HandSide side, XrHandJointEXT joint)
	{
		this->mHandSide = side;
		this->mTargetJoint = joint;
		return shared_from_this();
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

				// rotate a bit more so forward is kinda forward
				// Will use HMD position eventually
				diff = HOL::quaternionFromEulerAnglesDegrees(0, -35, 0) * diff;


				diff.z() = -diff.z();
				diff *= mMultiplier;
				diff.x() = std::clamp(diff.x(), -1.0f, 1.0f);
				diff.z() = std::clamp(diff.z(), -1.0f, 1.0f);

				// Joystick requires touch to be recognized
				this->submitInput(InputType::Touch, true);
				this->submitInput(InputType::XAxis, diff.x());
				this->submitInput(InputType::ZAxis, diff.z());
			}
		}
		else if (actionData.onUp)
		{
			this->submitInput(InputType::Touch, false);
			this->submitInput(InputType::XAxis, 0);
			this->submitInput(InputType::ZAxis, 0);
		}
	}
} // namespace HOL