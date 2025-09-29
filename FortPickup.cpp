#include "pch.h"
#include "FortPickup.h"

#include "UHook.h"

#include "FortInventory.h"
#include "World.h"

char FortPickup::hk_CompletePickupAnimation(AFortPickup* Pickup)
{
	if (!Pickup)
		return o_CompletePickupAnimation(Pickup);

	APlayerPawn_Athena_C* Pawn = reinterpret_cast<APlayerPawn_Athena_C*>(Pickup->PickupLocationData.PickupTarget);
	if (!Pawn)
		return o_CompletePickupAnimation(Pickup);

	AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Pawn->GetController());
	if (!PlayerController)
		return o_CompletePickupAnimation(Pickup);

	FGuid Swap = FGuid(-1, -1, -1, -1);

	FFortItemEntry* PickupEntry = &Pickup->PrimaryPickupItemEntry;
	FFortItemEntry* InvItemEntry = FortInventory::FindItem(PlayerController, Pickup->PickupLocationData.PickupGuid);

	if (!PickupEntry || !InvItemEntry)
	{
		SpawnPickup(PickupEntry->ItemDefinition, PickupEntry->Count, PickupEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);

		return o_CompletePickupAnimation(Pickup);
	}

	UFortWorldItemDefinition* PickupItemDefinition = reinterpret_cast<UFortWorldItemDefinition*>(PickupEntry->ItemDefinition);
	int IncomingCount = PickupEntry->Count;
	FGuid ItemGuid = PickupEntry->ItemGuid;

	if (!PickupItemDefinition)
		return o_CompletePickupAnimation(Pickup);

	if (FortInventory::GetQuickBars(PickupItemDefinition) == EFortQuickBars::Primary)
	{
		int NewCount = 0;
		int OverStack = 0;

		bool bSuccess = false;
		bool bDrop = false;

		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry* CurrentEntry = &PlayerController->WorldInventory->Inventory.ReplicatedEntries[i];
			if (CurrentEntry->ItemDefinition == PickupItemDefinition)
			{
				NewCount = CurrentEntry->Count + IncomingCount;
				OverStack = NewCount - PickupItemDefinition->MaxStackSize;

				CurrentEntry->Count = OverStack > 0 ? NewCount - OverStack : NewCount;

				PlayerController->WorldInventory->Inventory.MarkItemDirty(*CurrentEntry);
				PlayerController->WorldInventory->Inventory.MarkArrayDirty();

				if (OverStack <= 0)
				{
					bSuccess = true;
					break;
				}
			}
		}

		bDrop = !bSuccess;

		if (FortInventory::IsFull(PlayerController) && bDrop)
		{
			int DropCount = OverStack > 0 ? OverStack : IncomingCount;

			if (reinterpret_cast<UFortWorldItemDefinition*>(InvItemEntry->ItemDefinition)->bCanBeDropped && InvItemEntry->ItemDefinition->bAllowMultipleStacks)
			{
				PlayerController->ServerAttemptInventoryDrop(InvItemEntry->ItemGuid, InvItemEntry->Count);

				Swap = FortInventory::AddItem(PlayerController, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
			}
			else
			{
				SpawnPickup(PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
			}
		}
		else if (!FortInventory::IsFull(PlayerController) && bDrop)
		{
			int DropCount = OverStack > 0 ? OverStack : IncomingCount;

			FortInventory::AddItem(PlayerController, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
		}
	}
	else
		FortInventory::AddItem(PlayerController, PickupItemDefinition, IncomingCount, PickupEntry->LoadedAmmo);

	FGuid CurrentGuid = FGuid(-1, -1, -1, -1);
	if (Pawn->CurrentWeapon)
		CurrentGuid = Pawn->CurrentWeapon->ItemEntryGuid;

	if (UKismetGuidLibrary::NotEqual_GuidGuid(Swap, FGuid(-1, -1, -1, -1)) && UKismetGuidLibrary::EqualEqual_GuidGuid(CurrentGuid, Pickup->PickupLocationData.PickupGuid))
		PlayerController->ClientEquipItem(Swap);

	return o_CompletePickupAnimation(Pickup);
}

AFortPickupAthena* FortPickup::SpawnPickup(UFortItemDefinition* ItemDefinition, int OverrideCount, int LoadedAmmo, FVector Loc, bool bTossedFromContainer, AFortPawn* PawnWhoDroppedPickup)
{
	if (!ItemDefinition)
		return nullptr;

	FTransform SpawnTransform = FTransform();
	SpawnTransform.Scale3D = FVector(1, 1, 1);
	SpawnTransform.Translation = Loc;

	auto SpawnedPickup = World::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), SpawnTransform);
	if (!SpawnedPickup)
		return nullptr;

	FFortItemEntry* PickupEntry = &SpawnedPickup->PrimaryPickupItemEntry;
	PickupEntry->ItemDefinition = ItemDefinition;
	PickupEntry->Count = OverrideCount;
	PickupEntry->LoadedAmmo = LoadedAmmo;
	PickupEntry->ReplicationKey++;
	SpawnedPickup->OnRep_PrimaryPickupItemEntry();

	SpawnedPickup->bTossed = true;
	SpawnedPickup->TossPickup(Loc, nullptr, 1, true);

	SpawnedPickup->SetReplicateMovement(true);
	SpawnedPickup->MovementComponent = reinterpret_cast<UProjectileMovementComponent*>(UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), SpawnedPickup));

	SpawnedPickup->PawnWhoDroppedPickup = PawnWhoDroppedPickup;

	SpawnedPickup->bTossedFromContainer = bTossedFromContainer;
	SpawnedPickup->OnRep_TossedFromContainer();

	return SpawnedPickup;
}

void FortPickup::Patch()
{
	new UHook("UFortPickup::CompletePickupAnimation", 0xD01680, hk_CompletePickupAnimation, reinterpret_cast<void**>(&o_CompletePickupAnimation));
}

