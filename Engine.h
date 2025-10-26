#pragma once
class Engine
{
public:
	static float hk_GetMaxTickRate(UEngine* Context, float DeltaTime, bool bAllowFrameRateSmoothing);

	static void Patch();
};

