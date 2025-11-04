#pragma once
class BuildingSMActor
{
public:
	static inline void (*o_OnDamageServer)(ABuildingSMActor*, float, FGameplayTagContainer, FVector, FHitResult, AActor*, AActor*, FGameplayEffectContextHandle);
	static void hk_OnDamageServer(ABuildingSMActor* Context, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AActor* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext);

	static void Patch();
};

