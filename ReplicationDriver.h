#pragma once
#include "pch.h"

#include "LogMacros.h"

class ReplicationDriver
{
private:
	static inline UReplicationDriver* RepDriver;
	static inline void (*o_ServerReplicateActors)(UReplicationDriver*);
public:

	static bool ShouldSetServerReplicateActors() { return o_ServerReplicateActors == nullptr; }

	static void SetServerReplicateActors(UReplicationDriver* InRepDriver)
	{
		if (!InRepDriver)
		{
			UE_LOG("ReplicationDriver", "Warning", "Failed to set o_ServerReplicateActors as InRepDriver is null.");
			return;
		}

		if (o_ServerReplicateActors)
		{
			UE_LOG("ReplicationDriver", "Warning", "o_ServerReplicateActors is already set.");
			return;
		}

		void** RepDriverVFT = *reinterpret_cast<void***>(InRepDriver);
		o_ServerReplicateActors = decltype(o_ServerReplicateActors)(RepDriverVFT[0x53]);

		RepDriver = InRepDriver;
	}

	static void ServerReplicateActors() { if (!o_ServerReplicateActors) return;  o_ServerReplicateActors(RepDriver); }
};
