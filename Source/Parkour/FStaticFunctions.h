#pragma once

struct FStaticFunctions
{
public:
	template <typename T>
	static auto FindSmallestValue(TArray<T> Array);
	
	static float FindSmallestFloat(TArray<float> Array);
};