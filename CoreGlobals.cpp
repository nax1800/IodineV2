#include "pch.h"
#include "CoreGlobals.h"

bool bUseGameSessions = false;
const TCHAR* PlaylistPath = L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";

bool bIsPlaylistSet = false;
bool bIsServerListening = false;

AFortGameModeAthena* GetGameMode() { return reinterpret_cast<AFortGameModeAthena*>(UWorld::GetWorld()->AuthorityGameMode); }
AFortGameStateAthena* GetGameState() { return reinterpret_cast<AFortGameStateAthena*>(UWorld::GetWorld()->GameState); }