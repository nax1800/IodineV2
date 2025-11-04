#pragma once

class FortPlayerControllerAthena
{
public:
	static void hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* Pawn);

	static inline void (*o_ServerReadyToStartMatch)(AFortPlayerController*);
	static void hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* Context);

	static void hk_ServerExecuteInventoryItem(AFortPlayerControllerAthena* Context, FGuid ItemGuid);

	static void hk_ServerReturnToMainMenu(AFortPlayerControllerAthena* Context);
	static void hk_ServerPlayEmoteItem(AFortPlayerControllerAthena* Context, UFortMontageItemDefinitionBase* EmoteAsset);
	
	static void hk_ServerRemoveInventoryItem(AFortPlayerControllerAthena* Context, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval);
	static void hk_ServerItemWillBeDestroyed(AFortPlayerControllerAthena* Context, FGuid DestroyedItemGuid, int32 Count);

	static inline void (*o_EnterAircraft)(AFortPlayerControllerAthena* Context, AFortAthenaAircraft* Aircraft);
	static void hk_EnterAircraft(AFortPlayerControllerAthena* Context, AFortAthenaAircraft* Aircraft);

	static void hk_ServerAttemptInventoryDrop(AFortPlayerControllerAthena* Context, FGuid ItemGuid, int32 Count);

	static void hk_ServerCreateBuildingActor(AFortPlayerControllerAthena* Context, FBuildingClassData& BuildingClassData, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored);

	static void Patch();
};

