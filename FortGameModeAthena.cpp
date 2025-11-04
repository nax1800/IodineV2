#include "pch.h"
#include "FortGameModeAthena.h"

#include "UHook.h"

#include "LogMacros.h"
#include "CoreGlobals.h"
#include "UObjectGlobals.h"

#include "FortInventory.h"
#include "AbilitySystemComponent.h"

bool FortGameModeAthena::hk_ReadyToStartMatch(AFortGameModeAthena* Context)
{
	AFortGameStateAthena* GameState = reinterpret_cast<AFortGameStateAthena*>(Context->GameState);
	if (!GameState) return false;

	if (!GameState->CurrentPlaylistData)
	{
		UFortPlaylistAthena* Playlist = reinterpret_cast<UFortPlaylistAthena*>(StaticFindObject(UFortPlaylistAthena::StaticClass(), nullptr, PlaylistPath));
		if (!Playlist)
		{
			UE_LOG("GameMode", "Error", "Could not find playlist.");
			return false;
		}

		GameState->CurrentPlaylistData = Playlist;
		GameState->CurrentPlaylistId = Playlist->PlaylistId;
		GameState->OnRep_CurrentPlaylistData();
		GameState->OnRep_CurrentPlaylistId();

		Context->CurrentPlaylistId = Playlist->PlaylistId;
		Context->CurrentPlaylistName = Playlist->PlaylistName;

		UE_LOG("GameMode", "Info", "Playlist is set. ({})", Context->CurrentPlaylistName.ToString());

		Context->FriendlyFireType = Playlist->FriendlyFireType;
		Context->FortGameSession->MaxPlayers = Playlist->MaxPlayers;
		Context->FortGameSession->MaxPartySize = Playlist->MaxSocialPartySize;
	}

	if (!GameState->MapInfo) return false;

	if (!Context->bWorldIsReady)
	{
		Context->WarmupRequiredPlayerCount = 1;

		UWorld* World = UWorld::GetWorld();
		UNetDriver* NetDriver = UEngine::GetEngine()->CreateNetDriver(World, FName(282));
		if (!NetDriver) return false;

		World->NetDriver = NetDriver;
		NetDriver->World = World;
		NetDriver->NetDriverName = FName(282);

		FString Error;
		FURL URL = FURL();
		URL.Port = 7777;

		if (!NetDriver->InitListen(World, URL, false, Error)) return false;
		NetDriver->SetWorld(World);

		for (FLevelCollection& LevelCollection : World->LevelCollections) { LevelCollection.NetDriver = NetDriver; }

		UE_LOG("Server", "Info", "Listening on port {}.", URL.Port);

		Context->bWorldIsReady = true;
	}

	return o_ReadyToStartMatch(Context);
}

APawn* FortGameModeAthena::hk_SpawnDefaultPawnFor(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
	if (!NewPlayer || !StartSpot)
		return nullptr;

	return Context->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void FortGameModeAthena::hk_HandleStartingNewPlayer(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer)
{
	UE_LOG("GameMode", "Info", "HandleStartingNewPlayer called.");

	if (!NewPlayer)
		return o_HandleStartingNewPlayer(Context, NewPlayer);

	return o_HandleStartingNewPlayer(Context, NewPlayer);
}

void FortGameModeAthena::Patch()
{
	new UHook("FortGameModeAthena::ReadyToStartMatch", 0x25BAC60, hk_ReadyToStartMatch, reinterpret_cast<void**>(&o_ReadyToStartMatch));
	new UHook("FortGameModeAthena::SpawnDefaultPawnFor", 0xA083A0, hk_SpawnDefaultPawnFor);
	new UHook("FortGameModeAthena::HandleStartingNewPlayer", 0x25BA780, hk_HandleStartingNewPlayer, reinterpret_cast<void**>(&o_HandleStartingNewPlayer));
}

