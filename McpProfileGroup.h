#pragma once

struct FProfileRequestContext
{
    int GetStatus() { return *(int*)(__int64(this) + 0x60); }
    int SetStatus(int NewStatus) { *(int*)(__int64(this) + 0x60) = NewStatus; return GetStatus(); }
};

class McpProfileGroup
{
public:
     static inline void (*o_DispatchRequest)(UMcpProfileGroup* McpProfileGroup, FProfileRequestContext* Context, int a3);
     static void hk_DispatchRequest(UMcpProfileGroup* McpProfileGroup, FProfileRequestContext* Context, int a3);

     static void Patch();
};

