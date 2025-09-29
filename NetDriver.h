#pragma once

class NetDriver
{
public:
	static inline void (*o_TickFlush)(UNetDriver*, float);
	static void hk_TickFlush(UNetDriver* NetDriver, float DeltaSeconds);

	static void Patch();
};

