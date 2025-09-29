#include "pch.h"
#include "McpProfileGroup.h"

#include "LogMacros.h"
#include "UHook.h"

void McpProfileGroup::hk_DispatchRequest(UMcpProfileGroup* McpProfileGroup, FProfileRequestContext* Context, int a3)
{
    UE_LOG("Server", "Info", "DispatchRequest: McpProfileGroup: {}", reinterpret_cast<UObject*>(McpProfileGroup)->GetFullName());
    UE_LOG("Server", "Info", "DispatchRequest: Context.Status: {}", Context->GetStatus());

    Context->SetStatus(3);

    return o_DispatchRequest(McpProfileGroup, Context, 3);
}

void McpProfileGroup::Patch()
{
    new UHook("McpProfileGroup::DispatchRequest", 0x7F2370, hk_DispatchRequest, reinterpret_cast<void**>(&o_DispatchRequest));
}
