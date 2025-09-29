#pragma once
#include "LogMacros.h"

enum EHookStatus
{
	HS_FAILED = 0,
	HS_OK = 1,
	HS_INVALID = 2,
	HS_NOTFOUND = 3,
	HS_NONE = 4
};

struct FVFTHook
{
public:
	void** VFT = nullptr;
	uintptr_t Index = 0;
	LPVOID Detour;
	LPVOID* Original = nullptr;
	EHookStatus Status = HS_NONE;

public:
	bool IsValid() { return Index > 0x0 && VFT; }

	EHookStatus Create()
	{
		if (!VFT || !VFT[Index])
		{
			Status = HS_FAILED;
			return Status;
		}

		if (Original)
			*Original = VFT[Index];

		DWORD oldProtection;

		VirtualProtect(&VFT[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);
		VFT[Index] = Detour;
		VirtualProtect(&VFT[Index], 8, oldProtection, NULL);

		Status = HS_OK;
		return Status;
	}
};

struct FMHHook
{
public:
	uintptr_t Offset = 0;
	LPVOID Detour;
	LPVOID* Original = nullptr;
	EHookStatus Status = HS_NONE;

public:
	bool IsValid() { return Offset > 0x0; }
	inline EHookStatus Create()
	{
		if (Status != HS_NONE)
		{
			UE_LOG("UHook", "Error", "Failed to create MHHook, Reason: Already created.");
			Status = HS_FAILED;
			return Status;
		}

		if (!IsValid())
		{
			UE_LOG("UHook", "Error", "Failed to create MHHook, Reason: Offset is invalid.");
			Status = HS_INVALID;
			return Status;
		}

		MH_CreateHook(reinterpret_cast<void*>(InSDKUtils::GetImageBase() + Offset), Detour, Original);
		Status = MH_EnableHook(reinterpret_cast<void*>(InSDKUtils::GetImageBase() + Offset)) == MH_OK ? HS_OK : HS_FAILED;
		return Status;
	}
};

struct FExecHook
{
public:
	string Path = "";
	LPVOID Detour;
	LPVOID* Original = nullptr;
	EHookStatus Status = HS_NONE;
};

enum EHookType
{
	HT_MH = 0,
	HT_VFT = 1,
	HT_EXEC = 2,
	HT_UNKNOWN = 3
};

class UHook
{
public:
	UHook(string Name, uintptr_t Offset, LPVOID Detour, LPVOID* Original = nullptr)
	{
		pName = Name;

		FMHHook* MHHook = new FMHHook();
		MHHook->Offset = Offset;
		MHHook->Detour = Detour;
		MHHook->Original = Original;

		pHookType = EHookType::HT_MH;

		if (MHHook->Create() == HS_OK)
		{
			UE_LOG("UHook", "Info", "Created {} (MHHook).", this->pName);
		}

		this->pMH = MHHook;
	}

	UHook(string Name, void** VFT, uintptr_t Index, LPVOID Detour, LPVOID* Original = nullptr)
	{
		pName = Name;

		FVFTHook* VFTHook = new FVFTHook();
		VFTHook->VFT = VFT;
		VFTHook->Index = Index;
		VFTHook->Detour = Detour;
		VFTHook->Original = Original;

		pHookType = EHookType::HT_VFT;

		if (VFTHook->Create() == HS_OK)
		{
			UE_LOG("UHook", "Info", "Created {} (VFTHook).", this->pName);
		}

		this->pVFT = VFTHook;
	}

	UHook(string Name, string Path, LPVOID Detour, LPVOID* Original = nullptr)
	{
		pName = Name;

		FExecHook* ExecHook{};
		ExecHook->Path = Path;
		ExecHook->Detour = Detour;
		ExecHook->Original = Original;

		pHookType = EHookType::HT_EXEC;
	}

public:
	EHookType GetType();
	bool IsVFT();
	bool IsMH();
	bool IsExec();
	bool IsValid();

	FMHHook* GetMH();
	FVFTHook* GetVFT();
	FExecHook* GetExec();

private:
	string pName;
	FMHHook* pMH = nullptr;
	FVFTHook* pVFT = nullptr;
	FExecHook* pExec = nullptr;
	EHookType pHookType = HT_UNKNOWN;
};

inline vector<UHook*> Hooks{};

