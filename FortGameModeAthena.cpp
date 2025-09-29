#include "pch.h"
#include "FortGameModeAthena.h"

#include "UHook.h"

#include "LogMacros.h"
#include "CoreGlobals.h"
#include "UObjectGlobals.h"

#include "FortInventory.h"

bool FortGameModeAthena::hk_ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
	AFortGameStateAthena* GameState = reinterpret_cast<AFortGameStateAthena*>(GameMode->GameState);
	if (!GameState) return false;

	if (!bIsPlaylistSet)
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

		GameMode->CurrentPlaylistId = Playlist->PlaylistId;
		GameMode->CurrentPlaylistName = Playlist->PlaylistName;

		GameMode->FriendlyFireType = Playlist->FriendlyFireType;
		GameMode->FortGameSession->MaxPlayers = Playlist->MaxPlayers;
		GameMode->FortGameSession->MaxPartySize = Playlist->MaxSocialPartySize;

		bIsPlaylistSet = true;
	}

	if (!GameState->MapInfo)
		return false;

	if (!bIsServerListening)
	{
		GameMode->WarmupRequiredPlayerCount = 1;

		UWorld* World = UWorld::GetWorld();

		UNetDriver* NetDriver = reinterpret_cast<UNetDriver * (*)(UEngine*, UWorld*, FName)>(InSDKUtils::GetImageBase() + 0x2501480)(UEngine::GetEngine(), UWorld::GetWorld(), FName(282));
		if (!NetDriver)
			return false;

		World->NetDriver = NetDriver;
		NetDriver->World = World;
		NetDriver->NetDriverName = FName(282);

		FString Error;
		FURL URL = FURL();
		URL.Port = 7777;

		if (!reinterpret_cast<bool(*)(UNetDriver*, void*, FURL&, bool, FString&)>(InSDKUtils::GetImageBase() + 0x345650)(NetDriver, World, URL, false, Error))
			return false;

		bIsServerListening = true;

		reinterpret_cast<void(*)(UNetDriver*, UWorld*)>(InSDKUtils::GetImageBase() + 0x22B56F0)(NetDriver, World);

		for (FLevelCollection& LevelCollection : World->LevelCollections) { LevelCollection.NetDriver = NetDriver; }

		UE_LOG("Server", "Info", "Listening on port {}.", URL.Port);

		GameMode->bWorldIsReady = true;
	}

	return o_ReadyToStartMatch(GameMode);
}

APawn* FortGameModeAthena::hk_SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
	if (!NewPlayer || !StartSpot)
		return nullptr;

	return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void FortGameModeAthena::hk_HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer)
{
	if (!NewPlayer)
		return o_HandleStartingNewPlayer(GameMode, NewPlayer);

	return o_HandleStartingNewPlayer(GameMode, NewPlayer);
}

void FortGameModeAthena::Patch()
{
	new UHook("AFortGameModeAthena::ReadyToStartMatch", 0x25BAC60, hk_ReadyToStartMatch, reinterpret_cast<void**>(&o_ReadyToStartMatch));
	new UHook("AFortGameModeAthena::SpawnDefaultPawnFor", 0xA083A0, hk_SpawnDefaultPawnFor);
	new UHook("AFortGameModeAthena::HandleStartingNewPlayer", 0x25BA780, hk_HandleStartingNewPlayer, reinterpret_cast<void**>(&o_HandleStartingNewPlayer));
}

