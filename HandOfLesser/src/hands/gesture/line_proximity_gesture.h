#pragma once

#include "base_gesture.h"
#include "src/hands/sided_joint.h"
#include <HandOfLesserCommon.h>
#include <vector>

namespace HOL::Gesture::LineProximity
{
	struct Parameters
	{
		std::vector<HOL::SidedJoint> line1;
		std::vector<HOL::SidedJoint> line2;

		float mMinDistance;
		float mMaxDistance;
	};

	class Gesture : public BaseGesture::Gesture
	{
	public:
		Gesture() : BaseGesture::Gesture()
		{
			this->name = "LineProximityGesture";
		};
		static std::shared_ptr<Gesture> Create()
		{
			return std::make_shared<Gesture>();
		}

		LineProximity::Parameters parameters;

	private:

	protected:
		float evaluateInternal(GestureData data) override;
	};

} // namespace HOL::Gesture::LineProximity
