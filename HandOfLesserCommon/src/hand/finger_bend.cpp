#include "finger_bend.h"

float HOL::FingerBend::getCurlSum()
{
	float sum = 0;
	for (int i = 0; i < 3; i++)
	{
		sum += this->bend[i];
	}

	return sum;
}