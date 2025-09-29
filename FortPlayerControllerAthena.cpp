#include "pch.h"
#include "FortPlayerControllerAthena.h"

#include "UHook.h"
#include "CoreGlobals.h"
#include "FortInventory.h"

void FortPlayerControllerAthena::hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APlayerPawn_Athena_C* Pawn)
{
	if (!Pawn)
		return;

	PlayerController->AcknowledgedPawn = Pawn;

	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
	if (!PlayerState)
		return;

	FFortAthenaLoadout& CustomizationLoadout = PlayerController->CustomizationLoadout;

	if (!CustomizationLoadout.Character)
		return;

	if (!CustomizationLoadout.Character->HeroDefinition)
		return;

	Pawn->CustomizationLoadout = CustomizationLoadout;
	Pawn->OnRep_CustomizationLoadout();

	PlayerState->HeroType = CustomizationLoadout.Character->HeroDefinition;
	PlayerState->OnRep_HeroType();

	reinterpret_cast<void(*)(void*, void*)>(InSDKUtils::GetImageBase() + 0xF32360)(PlayerState, Pawn);
}

void FortPlayerControllerAthena::hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* PlayerController)
{
	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
	if (!PlayerState)
		return o_ServerReadyToStartMatch(PlayerController);

	//Abilities::ApplyAbilities(PlayerState);

	for (FItemAndCount& StartingItem : GetGameMode()->StartingItems)
	{
		FortInventory::AddItem(PlayerController, StartingItem.Item, StartingItem.Count);
	}

	FortInventory::AddItem(PlayerController, PlayerController->CustomizationLoadout.Pickaxe->WeaponDefinition);

	PlayerController->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), PlayerController));
	PlayerController->RecordMatchStats();
	PlayerController->RecordTeamStats();

	PlayerState->SquadId = (uint8)PlayerState->TeamIndex - 2;
	PlayerState->OnRep_PlayerTeam();
	PlayerState->OnRep_SquadId();

	PlayerState->WorldPlayerId = PlayerState->PlayerID;

	return o_ServerReadyToStartMatch(PlayerController);
}

void FortPlayerControllerAthena::hk_ServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
{
	if (PlayerController->IsInAircraft())
		return;

	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(PlayerController->Pawn);
	if (!Pawn)
		return;

	FFortItemEntry* FoundReplicatedEntry = FortInventory::FindItem(PlayerController, ItemGuid);
	if (!FoundReplicatedEntry)
		return;

	UFortWeaponItemDefinition* ItemDefinition = reinterpret_cast<UFortWeaponItemDefinition*>(FoundReplicatedEntry->ItemDefinition);
	if (!ItemDefinition)
		return;

	if (ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
	{
		if (UFortDecoItemDefinition* DecoDefinition = reinterpret_cast<UFortDecoItemDefinition*>(ItemDefinition))
		{
			Pawn->PickUpActor(nullptr, DecoDefinition);
			Pawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

			if (AFortDecoTool_ContextTrap* ContextTrapTool = reinterpret_cast<AFortDecoTool_ContextTrap*>(Pawn->CurrentWeapon))
				ContextTrapTool->ContextTrapItemDefinition = reinterpret_cast<UFortContextTrapItemDefinition*>(ItemDefinition);

			ItemDefinition = nullptr;
		}
	}

	Pawn->EquipWeaponDefinition(ItemDefinition, ItemGuid);
}

void FortPlayerControllerAthena::Patch()
{
	void** VFT = *reinterpret_cast<void***>(AAthena_PlayerController_C::GetDefaultObj());
	new UHook("FortPlayerControllerAthena::ServerAcknowledgePossession", VFT, 0x104, hk_ServerAcknowledgePossession);
	new UHook("FortPlayerControllerAthena::ServerReadyToStartMatch", VFT, 0x241, hk_ServerReadyToStartMatch, reinterpret_cast<void**>(&o_ServerReadyToStartMatch));
	new UHook("FortPlayerControllerAthena::ServerExecuteInventoryItem", VFT, 0x1E5, hk_ServerExecuteInventoryItem);
}
