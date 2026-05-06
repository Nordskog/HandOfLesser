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

				// Get reference forward/right from body tracking (chest if upper body tracking
				// active, head otherwise). Zero Y to lock the locomotion plane to horizontal,
				// avoiding pitch/roll affecting locomotion direction.
				const Eigen::Quaternionf& refQuat = gestureData.ReferenceOrientation;
				Eigen::Vector3f forward = refQuat * Eigen::Vector3f(0, 0, -1);
				forward.y() = 0.0f;
				if (forward.squaredNorm() > 1e-6f)
				{
					forward.normalize();
				}
				else
				{
					forward = Eigen::Vector3f(0, 0, -1);
				}
				Eigen::Vector3f right = forward.cross(Eigen::Vector3f::UnitY()).normalized();

				// Project diff onto reference axes
				float forwardAmount = diff.dot(forward);
				float rightAmount = diff.dot(right);

				forwardAmount *= mMultiplier;
				rightAmount *= mMultiplier;

				forwardAmount = std::clamp(forwardAmount, -1.0f, 1.0f);
				rightAmount = std::clamp(rightAmount, -1.0f, 1.0f);

				// Joystick requires touch to be recognized
				this->submitInput(InputType::Touch, true);
				this->submitInput(InputType::XAxis, rightAmount);
				this->submitInput(InputType::ZAxis, forwardAmount);
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