#include "FStaticFunctions.h"

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