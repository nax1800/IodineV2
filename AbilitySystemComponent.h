#pragma once
class AbilitySystemComponent
{
public:
	static FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle);
	static void hk_InternalServerTryActivateAbility(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData);

	static FGameplayAbilitySpec ConstructSpec(UGameplayAbility* InAbility, int32 InLevel = 1, int32 InInputID = -1, UObject* InSourceObject = nullptr);

	static FGameplayAbilitySpec* FindGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility);
	static void RemoveGameplayAbility(AFortPlayerStateAthena* PlayerState, UGameplayAbility* GameplayAbility);
	static void GrantGameplayAbility(AFortPlayerStateAthena* PlayerState, UClass* GameplayAbilityClass);
	static void ApplyAbilities(AFortPlayerStateAthena* PlayerState);

	static void Patch();
};

