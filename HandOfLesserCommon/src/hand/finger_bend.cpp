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

float HOL::FingerBend::getCurlSumWithoutDistal()
{
	float sum = 0;
	for (int i = 0; i < 2; i++)
	{
		sum += this->bend[i];
	}

	return sum;
}

void HOL::FingerBend::setSplay(float humanoidSplay)
{
	this->bend[FingerBendType::Splay] = humanoidSplay;
}