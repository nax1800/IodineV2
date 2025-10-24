#pragma once

class FortGameModeAthena
{
public:
	static inline bool (*o_ReadyToStartMatch)(AFortGameModeAthena*);
	static bool hk_ReadyToStartMatch(AFortGameModeAthena* Context);

	static APawn* hk_SpawnDefaultPawnFor(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot);

	static inline void (*o_HandleStartingNewPlayer)(AFortGameModeAthena*, AFortPlayerControllerAthena*);
	static void hk_HandleStartingNewPlayer(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer);

	static void Patch();
};

