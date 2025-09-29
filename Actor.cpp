#include "pch.h"
#include "Actor.h"

#include "UHook.h"

ENetMode Actor::hk_GetNetMode(AActor* Actor)
{
	return ENetMode::NM_DedicatedServer;
}

void Actor::Patch()
{
	// new UHook("AActor::GetNetMode", 0, hk_GetNetMode);
}
