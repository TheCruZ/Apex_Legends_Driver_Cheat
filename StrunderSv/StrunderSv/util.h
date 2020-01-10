#pragma once
#include "log.h"
#include <ntddk.h>
#include "eimport.h"
#include "structs.h"
#include <windef.h>
#include "JUNK.h"

typedef struct _BasicDataArea {
	BYTE NTName[4096];
	//BYTE buffer1[4096];
	BYTE IVDName[4096];
	//BYTE buffer3[4096];
	BYTE NTAddr[4096];
	//BYTE buffer5[4096];
	//BYTE buffer6[4096];
	//BYTE buffer7[4096];
	//BYTE buffer8[4096];
	//BYTE buffer9[4096];
	BYTE NTModListData[409600];
	BYTE ModListDataAddr[409600];
	//BYTE BigBuffer2[409600];
	//BYTE BigestBuffer0[4096000];
	//BYTE BigestBuffer1[4096000];
	//BYTE BigestBuffer2[4096000];
} BasicDataArea;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


extern BasicDataArea DataArea;
#ifdef __cplusplus
}
#endif // __cplusplus

void ClearDataArea(ULONG64 start, int type);
void SetDataArea(PVOID dataArea);
NTSTATUS StartThread(PKSTART_ROUTINE start_function, PVOID args);
BOOLEAN GetGlobalActive();
VOID SetGlobalActive(BOOLEAN st);
void Translate(unsigned char* buffer, size_t len);
void Sleep(int ms);
HANDLE FindProcessByName(wchar_t* name, size_t name_len);
NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);
NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);
BOOLEAN write(HANDLE pid, ULONG64 addr, ULONG64 data, ULONG64 size);
BOOLEAN read(HANDLE pid, ULONG64 addr, ULONG64 dest, ULONG64 size);
BOOLEAN fileExists(UNICODE_STRING path);