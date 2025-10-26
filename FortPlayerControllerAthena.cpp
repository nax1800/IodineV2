#include "pch.h"
#include "FortPlayerControllerAthena.h"

#include "UHook.h"
#include "CoreGlobals.h"
#include "FortInventory.h"

#include "AbilitySystemComponent.h"

void FortPlayerControllerAthena::hk_ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* Pawn)
{
	if (!Pawn)
		return;

	Context->AcknowledgedPawn = Pawn;

	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState);
	if (!PlayerState)
		return;

	FFortAthenaLoadout& CustomizationLoadout = Context->CustomizationLoadout;

	if (!CustomizationLoadout.Character)
		return;

	if (!CustomizationLoadout.Character->HeroDefinition)
		return;

	Pawn->CustomizationLoadout = CustomizationLoadout;
	Pawn->OnRep_CustomizationLoadout();

	PlayerState->HeroType = CustomizationLoadout.Character->HeroDefinition;
	PlayerState->OnRep_HeroType();
	PlayerState->ApplyCharacterCustomization(Pawn);
}

void FortPlayerControllerAthena::hk_ServerReadyToStartMatch(AFortPlayerControllerAthena* Context)
{
	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Context->PlayerState);
	if (!PlayerState)
		return o_ServerReadyToStartMatch(Context);

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
	if (Context->IsInAircraft())
		return;

	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(Context->Pawn);
	if (!Pawn)
		return;

	FFortItemEntry* FoundReplicatedEntry = FortInventory::FindItem(Context, ItemGuid);
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

	static UObject* AbilityToUse = UGAB_Emote_Generic_C::StaticClass()->DefaultObject;
	bool bShouldBeAbilityToUse = false;

	FGameplayAbilitySpec Spec = AbilitySystemComponent::ConstructSpec(reinterpret_cast<UGameplayAbility*>(AbilityToUse), 1, -1, EmoteAsset);

	AbilitySystemComponent->GiveAbilityAndActivateOnce(Spec);
}

void FortPlayerControllerAthena::hk_ServerRemoveInventoryItem(AFortPlayerControllerAthena* Context, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval)
{
	UE_LOG("PlayerController", "Info", "ServerRemoveInventoryItem Called.");
	if (!Context) return;

	FortInventory::RemoveItem(Context, ItemGuid, Count);
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
}
