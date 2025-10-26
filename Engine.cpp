#include "pch.h"
#include "Engine.h"

#include "UHook.h"

float Engine::hk_GetMaxTickRate(UEngine* Context, float DeltaTime, bool bAllowFrameRateSmoothing)
{
	return 30.0f;
}

void Engine::Patch()
{
	new UHook("Engine::GetMaxTickRate", 0x2508650, hk_GetMaxTickRate);
}
