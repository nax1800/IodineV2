#include "pch.h"
#include "GameSession.h"

#include "UHook.h"

bool GameSession::hk_KickPlayer(AFortGameSessionDedicated* GameSession, AFortPlayerControllerAthena* KickedPlayer, FText& KickReason) { return true; }

void GameSession::Patch()
{
	new UHook("GameSession::KickPlayer", 0x218A4A0, hk_KickPlayer);
}
