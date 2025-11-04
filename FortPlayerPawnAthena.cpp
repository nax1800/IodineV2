#include "pch.h"
#include "FortPlayerPawnAthena.h"

#include "UHook.h"

#include "FortInventory.h"
#include "AbilitySystemComponent.h"

void FortPlayerPawnAthena::hk_ServerHandlePickup(APlayerPawn_Athena_C* Context, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
{
	if (!Context || !Pickup || Pickup->bPickedUp) return;

	AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Context->Controller);
	if (!PlayerController) return;

	Context->IncomingPickups.Add(Pickup);
	FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
	PickupLocationData.StartDirection = reinterpret_cast<FVector_NetQuantizeNormal&>(InStartDirection);
	PickupLocationData.FlyTime = 0.40f;
	PickupLocationData.PickupTarget = Context;
	PickupLocationData.ItemOwner = Context;
	PickupLocationData.bPlayPickupSound = bPlayPickupSound;
	PickupLocationData.PickupGuid = Context->CurrentWeapon ? Context->CurrentWeapon->ItemEntryGuid : FGuid();
	Pickup->OnRep_PickupLocationData();

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void FortPlayerPawnAthena::hk_ServerHandlePickupWithSwap(APlayerPawn_Athena_C* Context, AFortPickup* Pickup, FGuid Swap, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
{
	if (!Context || !Pickup || Pickup->bPickedUp) return o_ServerHandlePickupWithSwap(Context, Pickup, Swap, InFlyTime, InStartDirection, bPlayPickupSound);

	AFortPlayerControllerAthena* Controller = reinterpret_cast<AFortPlayerControllerAthena*>(Context->GetController());
	if (!Controller) return o_ServerHandlePickupWithSwap(Context, Pickup, Swap, InFlyTime, InStartDirection, bPlayPickupSound);

	Context->IncomingPickups.Add(Pickup);

	FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
	PickupLocationData.PickupTarget = Context;
	PickupLocationData.FlyTime = 0.40f;
	PickupLocationData.ItemOwner = Context;
	PickupLocationData.PickupGuid = Swap;
	Pickup->OnRep_PickupLocationData();

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();

	return o_ServerHandlePickupWithSwap(Context, Pickup, Swap, InFlyTime, InStartDirection, bPlayPickupSound);
}

void FortPlayerPawnAthena::hk_ServerReviveFromDBNO(APlayerPawn_Athena_C* Context, AController* EventInstigator)
{
	if (!Context || !EventInstigator || !Context->IsDBNO())
		return;

	AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Context->Controller);
	if (!PlayerController || EventInstigator == PlayerController) return;

	AFortPlayerStateAthena* PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
	if (!PlayerState) return;

	AFortPlayerControllerAthena* InstigatorController = reinterpret_cast<AFortPlayerControllerAthena*>(EventInstigator);
	AFortPlayerStateAthena* InstigatorPlayerState = reinterpret_cast<AFortPlayerStateAthena*>(InstigatorController->PlayerState);
	AFortPlayerPawnAthena* InstigatorPawn = reinterpret_cast<AFortPlayerPawnAthena*>(InstigatorController->Pawn);

	if (!InstigatorPlayerState || !InstigatorPawn || PlayerState->TeamIndex != InstigatorPlayerState->TeamIndex) return;

	AbilitySystemComponent::RemoveGameplayAbility(PlayerState, UGAB_AthenaDBNO_C::GetDefaultObj());

	PlayerController->ClientOnPawnRevived(EventInstigator);

	Context->SetHealth(30.0f);

	Context->bPlayedDying = false;

	Context->bIsDBNO = false;
	Context->OnRep_IsDBNO();
	Context->DBNORevivalStacking = 0;

	FAthenaMatchStats KStats = InstigatorController->MatchReport->Stats;
	KStats.Revives++;
}

void FortPlayerPawnAthena::hk_NetMulticast_Athena_BatchedDamageCues(APlayerPawn_Athena_C* Context, FAthenaBatchedDamageGameplayCues BatchData)
{
	if (!Context) return o_NetMulticast_Athena_BatchedDamageCues(Context, BatchData);

	AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Context->Controller);
	if (!PlayerController) return o_NetMulticast_Athena_BatchedDamageCues(Context, BatchData);

	if (BatchData.HitActor && BatchData.HitActor->IsA(APlayerPawn_Athena_C::StaticClass()))
	{
		if (BatchData.bIsValid)
		{
			if (PlayerController->MatchReport)
			{
				if (BatchData.bIsCritical) PlayerController->MatchReport->Stats.CriticalHitsToHostiles++;
				else PlayerController->MatchReport->Stats.NormalHitsToHostiles++;
			}
		}
	}

	AFortWeapon* CurrentWeapon = Context->CurrentWeapon;
	if (CurrentWeapon)
	{
		FFortItemEntry* CurrentItemEntry = FortInventory::FindItem(PlayerController, CurrentWeapon->ItemEntryGuid);
		if (CurrentItemEntry)
		{
			CurrentItemEntry->LoadedAmmo = CurrentWeapon->AmmoCount;
			PlayerController->WorldInventory->Inventory.MarkItemDirty(*CurrentItemEntry);
			PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		}
	}

	return o_NetMulticast_Athena_BatchedDamageCues(Context, BatchData);
}

void FortPlayerPawnAthena::hk_OnCapsuleBeginOverlap(APlayerPawn_Athena_C* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult)
{
	if (!Context || !OtherActor || !OtherActor->IsA(AFortPickupAthena::StaticClass()) || Context->IsDBNO()) return o_OnCapsuleBeginOverlap(Context, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AFortPickupAthena* Pickup = reinterpret_cast<AFortPickupAthena*>(OtherActor);
	if (Pickup->PawnWhoDroppedPickup != Context)
	{
		UFortItemDefinition* ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;

		if (!ItemDefinition)
			return;

		if (FortInventory::GetQuickBars(ItemDefinition) != EFortQuickBars::Primary)
			Context->ServerHandlePickup(Pickup, 0.4f, FVector(), true);
	}

	return o_OnCapsuleBeginOverlap(Context, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void FortPlayerPawnAthena::Patch()
{
	void** VFT = *reinterpret_cast<void***>(APlayerPawn_Athena_C::GetDefaultObj());
	new UHook("FortPlayerPawnAthena::ServerHandlePickup", VFT, 0x18F, hk_ServerHandlePickup);
	new UHook("FortPlayerPawnAthena::ServerHandlePickup", VFT, 0x18D, hk_ServerHandlePickupWithSwap, reinterpret_cast<void**>(&o_ServerHandlePickupWithSwap));
	new UHook("FortPlayerPawnAthena::ServerReviveFromDBNO", VFT, 0x17D, hk_ServerReviveFromDBNO);

	new UHook("FortPlayerPawnAthena::NetMulticast_Athena_BatchedDamageCues", 0x11B2B10, hk_NetMulticast_Athena_BatchedDamageCues, reinterpret_cast<void**>(&o_NetMulticast_Athena_BatchedDamageCues));
	new UHook("FortPlayerPawnAthena::OnCapsuleBeginOverlap", 0xA34F40, hk_OnCapsuleBeginOverlap, reinterpret_cast<void**>(&o_OnCapsuleBeginOverlap));
}
