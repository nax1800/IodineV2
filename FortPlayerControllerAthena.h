#pragma once

class FortPlayerControllerAthena
{
public:
	static void hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* Pawn);

	static inline void (*o_ServerReadyToStartMatch)(AFortPlayerController*);
	static void hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* Context);

	static void hk_ServerExecuteInventoryItem(AFortPlayerControllerAthena* Context, FGuid ItemGuid);

	static void hk_ServerReturnToMainMenu(AFortPlayerControllerAthena* Context);
	static void hk_ServerPlayEmoteItem(AFortPlayerController* Context, UFortMontageItemDefinitionBase* EmoteAsset);

	static void Patch();
};

