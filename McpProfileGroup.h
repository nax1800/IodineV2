#pragma once

class McpProfileGroup
{
public:
     static inline void (*o_DispatchRequest)(UMcpProfileGroup* a1, __int64* a2, int a3);
     static void hk_DispatchRequest(UMcpProfileGroup* a1, __int64* a2, int a3);

     static void Patch();
};

