#include "pch.h"
#include "BuildingSMActor.h"

#include "UHook.h"
#include "CoreGlobals.h"
#include "UObjectGlobals.h"

#include "FortInventory.h"

void BuildingSMActor::hk_OnDamageServer(ABuildingSMActor* Context, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AActor* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext)
{
    if (!Context || !DamageCauser)
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    if (!DamageCauser->IsA(AFortWeapon::StaticClass()))
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    AFortWeapon* Weapon = reinterpret_cast<AFortWeapon*>(DamageCauser);
    if (!Weapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    AFortPlayerControllerAthena* PlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(InstigatedBy);
    if (!PlayerController)
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    APlayerPawn_Athena_C* PlayerPawn = reinterpret_cast<APlayerPawn_Athena_C*>(PlayerController->Pawn);
    if (!PlayerPawn)
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    if (Context->bDestroyed || Context->bPlayerPlaced)
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    bool bWeakSpotHit = (Damage == 100.f);

    UFortResourceItemDefinition* ResourceDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(Context->ResourceType);
    if (!ResourceDefinition)
        return o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    FCurveTableRowHandle BuildingResourceAmountOverride = Context->BuildingResourceAmountOverride;

    int ResourceAmount = 0;

    if (BuildingResourceAmountOverride.RowName.ComparisonIndex)
    {
        UCurveTable* ResourceRates = GetGameState()->CurrentPlaylistData->ResourceRates.Get();

        if (!ResourceRates)
            ResourceRates = reinterpret_cast<UCurveTable*>(StaticFindObject(UCurveTable::StaticClass(), nullptr, L"/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates"));

        float AmountOut = 0;

        UDataTableFunctionLibrary::EvaluateCurveTableRow(ResourceRates, BuildingResourceAmountOverride.RowName, 0.f, nullptr, &AmountOut, L"");

        float Amount = AmountOut / (Context->GetMaxHealth() / Damage);
        ResourceAmount = round(Amount);
    }

    if (Context->ResourceType == EFortResourceType::Wood)
        PlayerController->MatchReport->Stats.GatheredWood += ResourceAmount;
    else if (Context->ResourceType == EFortResourceType::Stone)
        PlayerController->MatchReport->Stats.GatheredStone += ResourceAmount;
    else if (Context->ResourceType == EFortResourceType::Metal)
        PlayerController->MatchReport->Stats.GatheredMetal += ResourceAmount;

    PlayerController->MatchReport->Stats.MaterialsGathered += ResourceAmount;

    PlayerController->ClientReportDamagedResourceBuilding(Context, Context->ResourceType, ResourceAmount, Context->bDestroyed, bWeakSpotHit);

    if (ResourceAmount > 0) FortInventory::AddItem(PlayerController, ResourceDefinition, ResourceAmount);

    o_OnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}

void BuildingSMActor::Patch()
{
    new UHook("BuildingSMActor::OnDamageServer", 0x1076EC0, hk_OnDamageServer, reinterpret_cast<LPVOID*>(&o_OnDamageServer));
}
