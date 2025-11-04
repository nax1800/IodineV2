#include "pch.h"
#include "NetDriver.h"

#include "ReplicationDriver.h"
#include "UHook.h"

void NetDriver::hk_TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
{
	if(!NetDriver || !NetDriver->ReplicationDriver)
		return o_TickFlush(NetDriver, DeltaSeconds);

	if (NetDriver->ClientConnections.Num() > 0 && !NetDriver->ClientConnections[0]->InternalAck)
	{
		NetDriver->ReplicationDriver->ServerReplicateActors(DeltaSeconds);

		if (GetAsyncKeyState(VK_F5) & 1) { UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"startaircraft", nullptr); }
	}

	return o_TickFlush(NetDriver, DeltaSeconds);
}

void NetDriver::Patch()
{
	new UHook("NetDriver::TickFlush", 0x22B6AB0, hk_TickFlush, reinterpret_cast<void**>(&o_TickFlush));
}
