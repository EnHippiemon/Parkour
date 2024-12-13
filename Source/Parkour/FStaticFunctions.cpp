#include "FStaticFunctions.h"

template <typename T>
auto FStaticFunctions::FindSmallestValue(TArray<T> Array)
{
	auto SmallestValue = Array[0];
	for (int i = 0; i < Array.Num(); ++i)
	{
		if (Array[i] < SmallestValue)
			SmallestValue = Array[i];
	}
	return SmallestValue;
}

float FStaticFunctions::FindSmallestFloat(TArray<float> Array)
{
	float SmallestValue = FLT_MAX;
	for (int i = 0; i < Array.Num(); ++i)
	{
		if (Array[i] < SmallestValue)
			SmallestValue = Array[i];
	}
	return SmallestValue;
}