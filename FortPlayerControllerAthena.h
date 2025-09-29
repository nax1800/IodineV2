#pragma once

class FortPlayerControllerAthena
{
public:
	static void hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APlayerPawn_Athena_C* Pawn);

	static inline void (*o_ServerReadyToStartMatch)(AFortPlayerController*);
	static void hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* PlayerController);

	static void hk_ServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid);

	static void Patch();
};

