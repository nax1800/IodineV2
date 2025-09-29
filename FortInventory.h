#pragma once

class FortInventory
{
public:
	static EFortQuickBars GetQuickBars(UFortItemDefinition* ItemDefinition);
	static bool IsFull(AFortPlayerController* PlayerController);
	static FFortItemEntry* FindItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition);
	static FFortItemEntry* FindItem(AFortPlayerController* PlayerController, FGuid ItemGuid);
	static UFortWorldItem* CreateItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count = 1);
	static bool RemoveItem(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count = -1);
	static FGuid AddItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0, bool bForceNewItem = false);
};

