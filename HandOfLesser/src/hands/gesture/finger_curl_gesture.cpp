#include "proximity_gesture.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "finger_curl_gesture.h"

namespace HOL::Gesture::FingerCurlGesture
{
	float Gesture::evaluateInternal(GestureData data)
	{
		/*
		printf("Curl: %.3f, %.3f, %.3f, %.3f\n",
			data.handPose[this->mSide]->fingers[FingerType::FingerIndex].getCurlSumWithoutDistal(),
			   data.handPose[this->mSide]->fingers[FingerType::FingerMiddle].getCurlSumWithoutDistal(),
			   data.handPose[this->mSide]->fingers[FingerType::FingerRing].getCurlSumWithoutDistal(),
			   data.handPose[this->mSide]->fingers[FingerType::FingerLittle].getCurlSumWithoutDistal()
		);
		*/

		float val = 0;
		int count = 0;
		if (this->parameters.first)
		{
			val += data.handPose[this->parameters.side]
					   ->fingers[this->parameters.finger]
					   .bend[FingerBendType::CurlFirst];
			count++;
		}
		if (this->parameters.second)
		{
			val += data.handPose[this->parameters.side]
					   ->fingers[this->parameters.finger]
					   .bend[FingerBendType::CurlSecond];
			count++;
		}
		if (this->parameters.third)
		{
			val += data.handPose[this->parameters.side]
					   ->fingers[this->parameters.finger]
					   .bend[FingerBendType::CurlThird];
			count++;
		}

		if (count <= 0)
			return 0;

		val /= (float)count;

		float rangeStart = HOL::degreesToRadians(this->parameters.minDegrees);
		float rangeEnd = HOL::degreesToRadians(this->parameters.maxDegrees);

		// ratio of val between start and end
		val = (val - rangeStart) / rangeEnd - rangeStart;

		return std::clamp(val, 0.f, 1.f);
	}

} // namespace HOL::Gesture::FingerCurlGesture
