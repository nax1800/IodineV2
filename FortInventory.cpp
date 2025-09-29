#include "pch.h"
#include "FortInventory.h"

#include "FortPickup.h"

EFortQuickBars FortInventory::GetQuickBars(UFortItemDefinition* ItemDefinition)
{
	if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
		return EFortQuickBars::Primary;

	return EFortQuickBars::Secondary;
}

bool FortInventory::IsFull(AFortPlayerController* PlayerController)
{
	int Slots = 0;
	TArray<FFortItemEntry>* InstancesPtr = &PlayerController->WorldInventory->Inventory.ReplicatedEntries;
	for (int i = 0; i < InstancesPtr->Num(); i++)
	{
		if (!InstancesPtr->operator[](i).ItemDefinition)
			continue;

		if (GetQuickBars(InstancesPtr->operator[](i).ItemDefinition) == EFortQuickBars::Primary)
		{
			Slots++;

			if (Slots >= 5)
			{
				break;
			}
		}
	}

	return Slots >= 5;
}

FFortItemEntry* FortInventory::FindItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition)
{
	TArray<FFortItemEntry> ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
	if (!ReplicatedEntries.IsValid())
		return nullptr;

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (ReplicatedEntries[i].ItemDefinition == ItemDefinition)
			return &ReplicatedEntries[i];
	}

	return nullptr;
}

FFortItemEntry* FortInventory::FindItem(AFortPlayerController* PlayerController, FGuid ItemGuid)
{
	TArray<FFortItemEntry> ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
	if (!ReplicatedEntries.IsValid())
		return nullptr;

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (UKismetGuidLibrary::EqualEqual_GuidGuid(ReplicatedEntries[i].ItemGuid, ItemGuid))
			return &ReplicatedEntries[i];
	}

	return nullptr;
}

UFortWorldItem* FortInventory::CreateItem(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count)
{
	UFortWorldItem* WorldItem = static_cast<UFortWorldItem*>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1));
	WorldItem->SetOwningControllerForTemporaryItem(PlayerController);
	return WorldItem;
}

bool FortInventory::RemoveItem(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count)
{
	if (!PlayerController)
		return false;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;
	bool bWasSuccessful = false;

	for (int i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (UKismetGuidLibrary::EqualEqual_GuidGuid(WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid, ItemGuid))
		{
			if (Count == -1 || Count >= WorldInventory->Inventory.ReplicatedEntries[i].Count)
			{
				WorldInventory->Inventory.ReplicatedEntries.Remove(i);
				break;
			}

			WorldInventory->Inventory.ReplicatedEntries[i].Count -= Count;
			WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[i]);
			bWasSuccessful = true;
		}
	}

	PlayerController->WorldInventory->Inventory.MarkArrayDirty();
	PlayerController->WorldInventory->HandleInventoryLocalUpdate();

	return bWasSuccessful;
}


FGuid FortInventory::AddItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, bool bForceNewItem)
{
	bool bAllowMultipleStacks = ItemDefinition->bAllowMultipleStacks;
	int32 MaxStackSize = ItemDefinition->MaxStackSize;
	TArray<FFortItemEntry>& ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
	UFortWorldItem* StackingItemInstance = nullptr;
	int OverStack = 0;

	if (MaxStackSize > 1)
	{
		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry CurrentEntry = ReplicatedEntries[i];
			UFortItemDefinition* CurrentReplicatedEntry = CurrentEntry.ItemDefinition;

			if (CurrentEntry.ItemDefinition == ItemDefinition)
			{
				if (CurrentEntry.Count < MaxStackSize || !bAllowMultipleStacks)
				{
					StackingItemInstance = reinterpret_cast<UFortWorldItem*>(CurrentReplicatedEntry);

					OverStack = CurrentEntry.Count + Count - MaxStackSize;

					if (!bAllowMultipleStacks && !(CurrentEntry.Count < MaxStackSize))
						break;

					int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

					FFortItemEntry* ReplicatedEntry = FindItem(PlayerController, CurrentEntry.ItemGuid);

					CurrentEntry.Count += AmountToStack;
					ReplicatedEntry->Count += AmountToStack;

					if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()))
					{
						FFortItemEntryStateValue Value{};
						Value.IntValue = 1;
						Value.StateType = EFortItemEntryState::ShouldShowItemToast;
						ReplicatedEntry->StateValues.Add(Value);
					}

					PlayerController->WorldInventory->Inventory.MarkItemDirty(CurrentEntry);
					PlayerController->WorldInventory->Inventory.MarkItemDirty(*ReplicatedEntry);

					if (OverStack <= 0)
						return ReplicatedEntry->ItemGuid;
				}
			}
		}
	}

	Count = OverStack > 0 ? OverStack : Count;

	if (OverStack > 0 && !ItemDefinition->bAllowMultipleStacks || OverStack > 0 && IsFull(PlayerController))
	{
		APlayerPawn_Athena_C* Pawn = static_cast<APlayerPawn_Athena_C*>(PlayerController->AcknowledgedPawn);

		if (!Pawn)
			return FGuid(-1, -1, -1, -1);

		FortPickup::SpawnPickup(ItemDefinition, Count, LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
		return FGuid(-1, -1, -1, -1);
	}

	UFortWorldItem* NewWorldItem = CreateItem(PlayerController, ItemDefinition, Count);
	if (!NewWorldItem)
		return FGuid(-1, -1, -1, -1);

	NewWorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;
	if (ItemDefinition->MaxStackSize < Count)
	{
		PlayerController->ServerAttemptInventoryDrop(NewWorldItem->ItemEntry.ItemGuid, Count - ItemDefinition->MaxStackSize);
		NewWorldItem->ItemEntry.Count = ItemDefinition->MaxStackSize;
	}

	if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()))
	{
		FFortItemEntryStateValue Value{};
		Value.IntValue = 1;
		Value.StateType = EFortItemEntryState::ShouldShowItemToast;
		NewWorldItem->ItemEntry.StateValues.Add(Value);
	}

	PlayerController->WorldInventory->Inventory.ReplicatedEntries.Add(NewWorldItem->ItemEntry);
	PlayerController->WorldInventory->HandleInventoryLocalUpdate();
	PlayerController->WorldInventory->Inventory.MarkArrayDirty();
	PlayerController->WorldInventory->Inventory.MarkItemDirty(NewWorldItem->ItemEntry);

	return NewWorldItem->ItemEntry.ItemGuid;
}
