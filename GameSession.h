#pragma once

class GameSession
{
public:
	static bool hk_KickPlayer(AFortGameSessionDedicated* GameSession, AFortPlayerControllerAthena* KickedPlayer, FText& KickReason);

	static void Patch();
};

