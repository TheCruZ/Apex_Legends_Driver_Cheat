#pragma once

#include <ntddk.h>

#ifdef __cplusplus
extern "C"
{
#endif

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
		IN PEPROCESS		SourceProcess,
		IN PVOID			SourceAddress,
		IN PEPROCESS		TargetProcess,
		IN PVOID			TargetAddress,
		IN SIZE_T			BufferSize,
		IN KPROCESSOR_MODE  PreviousMode,
		OUT PSIZE_T			ReturnSize
	);

	NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
		IN HANDLE			ProcessId,
		OUT PEPROCESS* Process
	);

	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(
		IN PEPROCESS		Process
	);

	NTSTATUS ZwQuerySystemInformation(
		ULONG InfoClass, 
		PVOID Buffer, 
		ULONG Length, 
		PULONG ReturnLength
	);
#ifdef __cplusplus
}
#endif
