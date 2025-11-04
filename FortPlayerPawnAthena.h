#pragma once
class FortPlayerPawnAthena
{
public:
	static void hk_ServerHandlePickup(APlayerPawn_Athena_C* Context, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);

	static inline void (*o_ServerHandlePickupWithSwap)(APlayerPawn_Athena_C* Context, AFortPickup* Pickup, FGuid Swap, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);
	static void hk_ServerHandlePickupWithSwap(APlayerPawn_Athena_C* Context, AFortPickup* Pickup, FGuid Swap, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);

	static void hk_ServerReviveFromDBNO(APlayerPawn_Athena_C* Context, AController* EventInstigator);

	static inline void (*o_NetMulticast_Athena_BatchedDamageCues)(APlayerPawn_Athena_C* Context, FAthenaBatchedDamageGameplayCues BatchData);
	static void hk_NetMulticast_Athena_BatchedDamageCues(APlayerPawn_Athena_C* Context, FAthenaBatchedDamageGameplayCues BatchData);

	static inline void (*o_OnCapsuleBeginOverlap)(APlayerPawn_Athena_C* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult);
	static void hk_OnCapsuleBeginOverlap(APlayerPawn_Athena_C* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult);

	static void Patch();
};

