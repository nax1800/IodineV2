#include "pch.h"
#include "McpProfileGroup.h"

#include "LogMacros.h"
#include "UHook.h"

void McpProfileGroup::hk_DispatchRequest(UMcpProfileGroup* a1, __int64* a2, int a3)
{
    UE_LOG("Server", "Info", "DispatchRequest: a1: {}", reinterpret_cast<UObject*>(a1)->GetFullName());
    UE_LOG("Server", "Info", "DispatchRequest: a2: {}", reinterpret_cast<UObject*>(a2)->GetFullName());

    *(int*)(__int64(a2) + 0x60) = 3;
    return o_DispatchRequest(a1, a2, 3);
}

void McpProfileGroup::Patch()
{
    new UHook("McpProfileGroup::DispatchRequest", 0x7F2370, hk_DispatchRequest, reinterpret_cast<void**>(&o_DispatchRequest));
}
