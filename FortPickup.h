#pragma once

class FortPickup
{
public:
	static inline char (*o_CompletePickupAnimation)(AFortPickup* Pickup);
	static char hk_CompletePickupAnimation(AFortPickup* Pickup);

	static AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDefinition, int OverrideCount, int LoadedAmmo, FVector Loc, bool bTossedFromContainer = false, AFortPawn* PawnWhoDroppedPickup = nullptr);

	static void Patch();
};

