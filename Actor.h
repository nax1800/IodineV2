#pragma once
#include "EngineBaseTypes.h"

class Actor
{
public:
	static ENetMode hk_GetNetMode(AActor* Actor);

	static void Patch();
};

