#include "pch.h"

#include "CoreGlobals.h"
#include "LogMacros.h"

#include "NetDriver.h"
#include "FortGameModeAthena.h"
#include "FortPlayerControllerAthena.h"
#include "FortPlayerPawnAthena.h"
#include "FortWeapon.h"
#include "GameSession.h"
#include "FortPickup.h"
#include "World.h"
#include "Engine.h"
#include "BuildingSMActor.h"

#include "McpProfileGroup.h"
#include "AbilitySystemComponent.h"

#include "UHook.h"

void hkCollectGarbage() { return; }


DWORD MainThread(LPVOID)
{
#ifdef LOGGING
    AllocConsole();
    FILE* File;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SetConsoleTitle(L"IodineV2 - Log");
#endif // LOGGING

    void** VFT = *reinterpret_cast<void***>(UFortKismetLibrary::GetDefaultObj());
    UE_LOG("Launch", "Info", "FortKismetLibrary VFT: 0x{:x}", reinterpret_cast<uintptr_t>(VFT) - InSDKUtils::GetImageBase());

    Sleep(5000);

    if (MH_Initialize() != MH_OK)
    {
        UE_LOG("Launch", "Error", "MinHook failed to initialize.");
        ExitThread(1);
    }

    *(uint8_t*)((InSDKUtils::GetImageBase() + 0x255BB17) + 7) = 0x74;
    UE_LOG("Launch", "Info", "Matchmaking should now be supported.");

    *(bool*)(InSDKUtils::GetImageBase() + 0x4A9CA14) = false;

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

    for (uintptr_t FuncToNull : vector<uintptr_t>{ 0xA767B0, 0xC22E90, 0x1BEE130, 0x22EFC10 })
    {
        uintptr_t func = InSDKUtils::GetImageBase() + FuncToNull;
        DWORD dwProtection;
        VirtualProtect((PVOID)func, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
        *(uint8_t*)func = 0xC3;
        DWORD dwTemp;
        VirtualProtect((PVOID)func, 1, dwProtection, &dwTemp);
    }

    uint8_t* ByteToPatch = (uint8_t*)(uint8_t*)(InSDKUtils::GetImageBase() + 0x9EB786);
    DWORD dwProtection;
    VirtualProtect((PVOID)ByteToPatch, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
    *ByteToPatch = 0x85;
    DWORD dwTemp;
    VirtualProtect((PVOID)ByteToPatch, 1, dwProtection, &dwTemp);

    NetDriver::Patch();
    FortGameModeAthena::Patch();
    FortPlayerControllerAthena::Patch();
    FortPlayerPawnAthena::Patch();
    FortWeapon::Patch();
    GameSession::Patch();
    FortPickup::Patch();
    World::Patch();
    Engine::Patch();
    BuildingSMActor::Patch();

    McpProfileGroup::Patch();
    AbilitySystemComponent::Patch();

    new UHook("CollectGarbage", 0x25151D0, hkCollectGarbage);

    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, MainThread, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

