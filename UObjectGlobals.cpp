#include "pch.h"
#include "UObjectGlobals.h"

UObject* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass) { return reinterpret_cast<UObject * (*)(UClass*, void*, const wchar_t*, bool)>(InSDKUtils::GetImageBase() + 0x14e5890)(Class, InOuter, Name, ExactClass); }
