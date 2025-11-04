#include "pch.h"
#include "FortPlayerControllerAthena.h"

#include "UHook.h"
#include "CoreGlobals.h"
#include "UObjectGlobals.h"

#include "FortInventory.h"
#include "FortPickup.h"

#include "AbilitySystemComponent.h"
#include "World.h"

void FortPlayerControllerAthena::hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* Pawn)
{
	if (!Pawn) return;

	Context->AcknowledgedPawn = Pawn;

	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState);
	if (!PlayerState) return;

	FFortAthenaLoadout& CustomizationLoadout = Context->CustomizationLoadout;

	if (!CustomizationLoadout.Character || !CustomizationLoadout.Character->HeroDefinition) return;

	Pawn->CustomizationLoadout = CustomizationLoadout;
	Pawn->OnRep_CustomizationLoadout();

	PlayerState->HeroType = CustomizationLoadout.Character->HeroDefinition;
	PlayerState->OnRep_HeroType();
	PlayerState->ApplyCharacterCustomization(Pawn);
}

void FortPlayerControllerAthena::hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* Context)
{
	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState);
	if (!PlayerState) return o_ServerReadyToStartMatch(Context);

	AbilitySystemComponent::ApplyAbilities(PlayerState);

	for (FItemAndCount& StartingItem : GetGameMode()->StartingItems) { FortInventory::AddItem(Context, StartingItem.Item, StartingItem.Count); }
	FortInventory::AddItem(Context, Context->CustomizationLoadout.Pickaxe->WeaponDefinition);

	static UFortWeaponRangedItemDefinition* ScarItemDefinition = reinterpret_cast<UFortWeaponRangedItemDefinition*>(StaticFindObject(UFortWeaponRangedItemDefinition::StaticClass(), nullptr, L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03"));
	FortInventory::AddItem(Context, ScarItemDefinition);
	FortInventory::AddItem(Context, ScarItemDefinition->GetAmmoWorldItemDefinition_BP(), 999);

	Context->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), Context));
	Context->RecordMatchStats();
	Context->RecordTeamStats();

	PlayerState->SquadId = (uint8)PlayerState->TeamIndex - 2;
	PlayerState->OnRep_PlayerTeam();
	PlayerState->OnRep_SquadId();

	PlayerState->WorldPlayerId = PlayerState->PlayerID;

	return o_ServerReadyToStartMatch(Context);
}

void FortPlayerControllerAthena::hk_ServerExecuteInventoryItem(AFortPlayerControllerAthena* Context, FGuid ItemGuid)
{
	if (Context->IsInAircraft()) return;

	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(Context->Pawn);
	if (!Pawn) return;

	FFortItemEntry* FoundReplicatedEntry = FortInventory::FindItem(Context, ItemGuid);
	if (!FoundReplicatedEntry) return;

	UFortWeaponItemDefinition* ItemDefinition = reinterpret_cast<UFortWeaponItemDefinition*>(FoundReplicatedEntry->ItemDefinition);
	if (!ItemDefinition) return;

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

void FortPlayerControllerAthena::hk_ServerReturnToMainMenu(AFortPlayerControllerAthena* Context)
{
	if (!Context) return;

	Context->ClientReturnToMainMenu(L"");
}

void FortPlayerControllerAthena::hk_ServerPlayEmoteItem(AFortPlayerControllerAthena* Context, UFortMontageItemDefinitionBase* EmoteAsset)
{
	if (!Context) return;

	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState);
	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(Context->Pawn);

	if (!EmoteAsset || !PlayerState || !Pawn)
		return;

	UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

	UObject* AbilityToUse = nullptr;
	bool bShouldBeAbilityToUse = false;

	if (!AbilityToUse)
		AbilityToUse = UGAB_Emote_Generic_C::StaticClass()->DefaultObject;

	FGameplayAbilitySpec Spec = AbilitySystemComponent::ConstructSpec(reinterpret_cast<UGameplayAbility*>(AbilityToUse), 1, -1, EmoteAsset);

	AbilitySystemComponent->GiveAbilityAndActivateOnce(Spec);
}

void FortPlayerControllerAthena::hk_ServerRemoveInventoryItem(AFortPlayerControllerAthena* Context, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval)
{
	UE_LOG("PlayerController", "Info", "ServerRemoveInventoryItem Called.");
	if (!Context) return;

	FortInventory::RemoveItem(Context, ItemGuid, Count);
}

void FortPlayerControllerAthena::hk_ServerItemWillBeDestroyed(AFortPlayerControllerAthena* Context, FGuid DestroyedItemGuid, int32 Count)
{
	UE_LOG("PlayerController", "Info", "ServerItemWillBeDestroyed Called.");
	if (!Context) return;

	FortInventory::RemoveItem(Context, DestroyedItemGuid, Count);
}

void FortPlayerControllerAthena::hk_EnterAircraft(AFortPlayerControllerAthena* Context, AFortAthenaAircraft* Aircraft)
{
	if (!Context) return o_EnterAircraft(Context, Aircraft);

	TArray<FFortItemEntry> ReplicatedEntries = Context->WorldInventory->Inventory.ReplicatedEntries;
	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (reinterpret_cast<UFortWorldItemDefinition*>(ReplicatedEntries[i].ItemDefinition)->bCanBeDropped)
			FortInventory::RemoveItem(Context, ReplicatedEntries[i].ItemGuid);
	}

	return o_EnterAircraft(Context, Aircraft);
}

void FortPlayerControllerAthena::hk_ServerAttemptInventoryDrop(AFortPlayerControllerAthena* Context, FGuid ItemGuid, int32 Count)
{
	if (!Context) return;

	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(Context->Pawn);
	if (!Pawn) return;

	FFortItemEntry* ReplicatedEntry = FortInventory::FindItem(Context, ItemGuid);

	if (!ReplicatedEntry || Count > ReplicatedEntry->Count) return;

	AFortPickupAthena* SpawnedPickup = FortPickup::SpawnPickup(ReplicatedEntry->ItemDefinition, Count, ReplicatedEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
	if (!SpawnedPickup) return;

	FortInventory::RemoveItem(Context, ItemGuid, Count);
}

void FortPlayerControllerAthena::hk_ServerCreateBuildingActor(AFortPlayerControllerAthena* Context, FBuildingClassData& BuildingClassData, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored)
{
	auto Class = BuildingClassData.BuildingClass.Get();
	TArray<AActor*> BuildingActorsToDestroy;
	char Result;

	static auto CantBuildFunc = reinterpret_cast<bool(__fastcall*)(UWorld*, UObject*, FVector, FRotator, char, void*, char*)>(InSDKUtils::GetImageBase() + 0xC37840);
	if (!CantBuildFunc(UWorld::GetWorld(), Class, BuildLoc, BuildRot, bMirrored, &BuildingActorsToDestroy, &Result))
	{
		for (int i = 0; i < BuildingActorsToDestroy.Num(); i++)
		{
			BuildingActorsToDestroy[i]->K2_DestroyActor();
		}

		FTransform Transform{};
		Transform.Translation = BuildLoc;
		Transform.Rotation = World::FRotToQuat(BuildRot);
		Transform.Scale3D = { 1, 1, 1 };

		if (auto NewBuilding = World::SpawnActor<ABuildingSMActor>(Class, Transform))
		{
			NewBuilding->bPlayerPlaced = true;
			NewBuilding->SetOwner(Context);
			NewBuilding->Team = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState)->TeamIndex;
			NewBuilding->SetMirrored(bMirrored);
			NewBuilding->OnRep_Team();
			NewBuilding->InitializeKismetSpawnedBuildingActor(NewBuilding, Context);

			if (NewBuilding->ResourceType == EFortResourceType::Wood)
				Context->MatchReport->Stats.UsedWood += 10;
			else if (NewBuilding->ResourceType == EFortResourceType::Stone)
				Context->MatchReport->Stats.UsedStone += 10;
			else if (NewBuilding->ResourceType == EFortResourceType::Metal)
				Context->MatchReport->Stats.UsedMetal += 10;

			Context->MatchReport->Stats.MaterialsUsed += 10;

			if (!Context->bBuildFree)
			{
				FFortItemEntry* Entry = FortInventory::FindItem(Context, UFortKismetLibrary::K2_GetResourceItemDefinition(NewBuilding->ResourceType));
				if (Entry) FortInventory::RemoveItem(Context, Entry->ItemGuid, 10);
			}
		}
	}

	BuildingActorsToDestroy.Free();
}

void FortPlayerControllerAthena::Patch()
{
	void** VFT = *reinterpret_cast<void***>(AAthena_PlayerController_C::GetDefaultObj());
	new UHook("FortPlayerControllerAthena::ServerAcknowledgePossession", VFT, 0x104, hk_ServerAcknowledgePossession);
	new UHook("FortPlayerControllerAthena::ServerReadyToStartMatch", VFT, 0x241, hk_ServerReadyToStartMatch, reinterpret_cast<void**>(&o_ServerReadyToStartMatch));
	new UHook("FortPlayerControllerAthena::ServerExecuteInventoryItem", VFT, 0x1E5, hk_ServerExecuteInventoryItem);
	new UHook("FortPlayerControllerAthena::ServerReturnToMainMenu", VFT, 0x23D, hk_ServerReturnToMainMenu);
	new UHook("FortPlayerControllerAthena::ServerPlayEmoteItem", VFT, 0x1B3, hk_ServerPlayEmoteItem);
	new UHook("FortPlayerControllerAthena::ServerRemoveInventoryItem", VFT, 0x1E1, hk_ServerRemoveInventoryItem);
	new UHook("FortPlayerControllerAthena::ServerItemWillBeDestroyed", VFT, 0x1F2, hk_ServerItemWillBeDestroyed);
	new UHook("FortPlayerControllerAthena::ServerAttemptInventoryDrop", VFT, 0x1F7, hk_ServerAttemptInventoryDrop);
	new UHook("FortPlayerControllerAthena::ServerCreateBuildingActor", VFT, 0x201, hk_ServerCreateBuildingActor);

	new UHook("FortPlayerControllerAthena::EnterAircraft", 0xA26D10, hk_EnterAircraft, reinterpret_cast<void**>(&o_EnterAircraft));
}
