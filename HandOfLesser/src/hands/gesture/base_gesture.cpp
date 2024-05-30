#include "base_gesture.h"

using namespace HOL;

float HOL::BaseGesture::evaluate(GestureData data)
{
	return this->lastValue = this->evaluateInternal(data);
}

std::vector<std::shared_ptr<BaseGesture>>& HOL::BaseGesture::getSubGestures()
{
	return this->mSubGestures;
}