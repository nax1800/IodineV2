#include "pch.h"

#include "CoreGlobals.h"
#include "LogMacros.h"

#include "NetDriver.h"
#include "FortGameModeAthena.h"
#include "FortPlayerControllerAthena.h"
#include "GameSession.h"
#include "FortPickup.h"
#include "World.h"

#include "UHook.h"

void hkCollectGarbage() { return; }

inline void (*o_DispatchRequest)(UMcpProfileGroup* a1, __int64* a2, int a3);
inline void hk_DispatchRequest(UMcpProfileGroup* a1, __int64* a2, int a3)
{
    UE_LOG("Server", "Info", "DispatchRequest: a1: {}", reinterpret_cast<UObject*>(a1)->GetFullName());

    *(int*)(__int64(a2) + 0x60) = 3;
    return o_DispatchRequest(a1, a2, 3);
}


DWORD MainThread(LPVOID)
{
#ifdef LOGGING
    AllocConsole();
    FILE* File;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SetConsoleTitle(L"IodineV2 - Log");
#endif // LOGGING

    Sleep(5000);

    if (MH_Initialize() != MH_OK)
    {
        UE_LOG("Windows", "Error", "MinHook failed to initialize.");
        ExitThread(1);
    }

    if (!bUseGameSessions)
    {
        UE_LOG("Server", "Info", "NOT using Game Sessions.");

        *(uint8_t*)((InSDKUtils::GetImageBase() + 0x255BB17) + 7) = 0x74;
        UE_LOG("Server", "Info", "Matchmaking should now be supported.");
    }

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);

    for (uintptr_t FuncToNull : vector<uintptr_t>{ 0xA767B0, 0xC22E90, 0xF1C000 })
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

    *(bool*)(InSDKUtils::GetImageBase() + 0x4A9CA14) = false;

    NetDriver::Patch();
    FortGameModeAthena::Patch();
    FortPlayerControllerAthena::Patch();
    GameSession::Patch();
    FortPickup::Patch();
    World::Patch();

    new UHook("MCP::DispatchRequest", 0x7F2370, hk_DispatchRequest, reinterpret_cast<void**>(&o_DispatchRequest));
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

