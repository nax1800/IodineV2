#pragma once
#include "EngineBaseTypes.h"

struct FActorSpawnParameters
{
	FName Name = FName(0);
	AActor* Template = nullptr;
	AActor* Owner = nullptr;
	APawn* Instigator = nullptr;
	ULevel* OverrideLevel = nullptr;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	uint16	bRemoteOwned : 1;
	uint16	bNoFail : 1;
	uint16	bDeferConstruction : 1;
	uint16	bAllowDuringConstructionScript : 1;
	EObjectFlags ObjectFlags;
};

class World
{
public:
	static void sinCos(float* ScalarSin, float* ScalarCos, float Value);
	static FQuat FRotToQuat(FRotator Rot);

	static FActorSpawnParameters CreateSpawnParams(ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, bool bDeferConstruction = false, AActor* Owner = nullptr);

	template<typename T = AActor>
	static inline T* SpawnActor(UClass* ActorClass, FTransform SpawnTransform = FTransform(), FActorSpawnParameters SpawnParameters = CreateSpawnParams())
	{
		static auto oSpawnActor = (AActor * (*)(UWorld * World, UClass * Class, FTransform const* UserTransformPtr, const FActorSpawnParameters & SpawnParameters))(InSDKUtils::GetImageBase() + 0x22456D0);
		T* Actor = reinterpret_cast<T*>(oSpawnActor(UWorld::GetWorld(), ActorClass, &SpawnTransform, SpawnParameters));
		return Actor;
	}

	template<typename T = AActor>
	static inline T* SpawnActor(UClass* ActorClass, FVector Location = {}, FRotator Rotation = {}, FActorSpawnParameters SpawnParameters = CreateSpawnParams())
	{
		FTransform SpawnTransform{};
		SpawnTransform.Translation = Location;
		SpawnTransform.Scale3D = { 1,1,1 };
		SpawnTransform.Rotation = FRotToQuat(Rotation);

		T* Actor = SpawnActor<T>(ActorClass, SpawnTransform, SpawnParameters);

		VirtualFree(&SpawnParameters, 0, MEM_RELEASE);

		return Actor;
	}

	static ENetMode hk_GetNetMode(UWorld* World);

	static void Patch();
};

