#include "pch.h"
#include "UHook.h"

EHookType UHook::GetType() { return this->pHookType; }

bool UHook::IsMH() { return GetType() == EHookType::HT_MH; }
bool UHook::IsVFT() { return GetType() == EHookType::HT_VFT; }
bool UHook::IsExec() { return GetType() == EHookType::HT_EXEC; }
bool UHook::IsValid() { return (this->pMH || this->pExec || this->pVFT); }

FMHHook* UHook::GetMH()
{
	if (!this->pMH)
		UE_LOG("UHook", "Warning", "{} has an invalid MH.", this->pName);

	return this->pMH;
}

FVFTHook* UHook::GetVFT()
{
	if (!this->pVFT)
		UE_LOG("UHook", "Warning", "{} has an invalid VFT.", this->pName);

	return this->pVFT;
}

FExecHook* UHook::GetExec()
{
	if (!this->pExec)
		UE_LOG("UHook", "Warning", "{} has an invalid Exec.", this->pName);

	return this->pExec;
}
