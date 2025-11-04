#include "pch.h"
#include "FortWeapon.h"

#include "UHook.h"

#include "FortInventory.h"

void FortWeapon::hk_UseSpareAmmo(AFortWeapon* Context, int AmountToUse)
{
	if (!Context || !Context->WeaponData)
		return;

	APlayerPawn_Athena_C* Owner = reinterpret_cast<APlayerPawn_Athena_C*>(Context->GetOwner());
	if (!Owner) return;

	AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Owner->Controller);
	if (!PlayerController) return;

	UFortWorldItemDefinition* AmmoDef = Context->WeaponData->GetAmmoWorldItemDefinition_BP();
	if (!AmmoDef) return;

	FFortItemEntry* ReplicatedEntry = FortInventory::FindItem(PlayerController, AmmoDef);
	if (!ReplicatedEntry) return;

	FortInventory::RemoveItem(PlayerController, ReplicatedEntry->ItemGuid, AmountToUse);
}

void FortWeapon::Patch()
{
	new UHook("FortWeapon::UseSpareAmmo", 0x1069200, hk_UseSpareAmmo);
}
