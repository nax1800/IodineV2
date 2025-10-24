#include "pch.h"
#include "AbilitySystemComponent.h"

#include "UHook.h"

#include "UObjectGlobals.h"

FGameplayAbilitySpec* AbilitySystemComponent::FindAbilitySpecFromHandle(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle)
{
    for (int i = 0; i < Context->ActivatableAbilities.Items.Num(); i++)
    {
        if (Context->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle)
        {
            return &Context->ActivatableAbilities.Items[i];
        }
    }

    return nullptr;
}

void AbilitySystemComponent::hk_InternalServerTryActivateAbility(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Context, Handle);
    if (!Spec)
    {
        Context->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }

    const UGameplayAbility* AbilityToActivate = Spec->Ability;

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (Context->InternalTryActivateAbility(Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
    }
    else
    {
        Context->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
        Context->ActivatableAbilities.MarkItemDirty(*Spec);
    }
}

FGameplayAbilitySpec* AbilitySystemComponent::FindGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility)
{
    UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;
    if (!AbilitySystemComponent) return nullptr;

    for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
    {
        auto& Spec = AbilitySystemComponent->ActivatableAbilities.Items[i];
        if (!Spec.Ability) continue;

        if (Spec.Ability == GameplayAbility) return &Spec;
    }

    return nullptr;
}

void AbilitySystemComponent::RemoveGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility)
{
    if (!GameplayAbility)
        return;

    UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;
    if (!AbilitySystemComponent)
        return;

    FGameplayAbilitySpec* AbilitySpec = FindGameplayAbility(PlayerState, GameplayAbility);

    if (!AbilitySpec) return;

    AbilitySystemComponent->ClientCancelAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
    AbilitySystemComponent->ClientEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
    AbilitySystemComponent->ServerEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo, {});
}

void AbilitySystemComponent::GrantGameplayAbility(AFortPlayerStateAthena* PlayerState, UClass* GameplayAbilityClass)
{
    UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

    if (!AbilitySystemComponent)
        return;

    FGameplayAbilitySpec NewSpec{};

    static auto FGameplayAbilitySpec_Construct = reinterpret_cast<void (*)(const struct FGameplayAbilitySpec*, class UObject*, int, int, class UObject*)>(InSDKUtils::GetImageBase() + 0x103da30);
    FGameplayAbilitySpec_Construct(&NewSpec, reinterpret_cast<UGameplayAbility*>(GameplayAbilityClass->DefaultObject), 1, -1, nullptr);

    AbilitySystemComponent->GiveAbility(NewSpec);
    return;
}

void AbilitySystemComponent::ApplyAbilities(AFortPlayerStateAthena* PlayerState)
{
    static auto AbilitySet = reinterpret_cast<UFortAbilitySet*>(StaticFindObject(UFortAbilitySet::StaticClass(), nullptr, L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer"));
    TArray<TSubclassOf<UFortGameplayAbility>> GameplayAbilities = AbilitySet->GameplayAbilities;
    for (int i = 0; i < GameplayAbilities.Num(); i++) { GrantGameplayAbility(PlayerState, GameplayAbilities[i].Get()); }
}

void AbilitySystemComponent::Patch()
{
    void** VFT = *reinterpret_cast<void***>(UFortAbilitySystemComponentAthena::GetDefaultObj());
    new UHook("AbilitySystemComponent::InternalServerTryActivateAbility", VFT, 0xCB, hk_InternalServerTryActivateAbility);
}