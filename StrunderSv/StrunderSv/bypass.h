#pragma once

#include <ntddk.h>
#include <windef.h>
#include "structs.h"
#include "eimport.h"
#include "JUNK.h"
#include "log.h"
#include "util.h"



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _NTOSAddrs {
	uintptr_t StartAddress;
	uintptr_t Size;
} NTOSAddrs;

UINT64 ClearCacheEntry(UNICODE_STRING name);
uintptr_t SearchNtoskrnlPattern(BYTE* bMask, char* szMask, NTOSAddrs* ntos);
#ifdef __cplusplus
}
#endif // __cplusplus