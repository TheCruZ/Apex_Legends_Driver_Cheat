#include "util.h"

BOOLEAN GlobalActive = TRUE;

const char* original = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/\\";
const char* modified = "qpDWJNvXkyagT/ULHKPxumjIcGloZbhiRVOEsteQwAFr\\SBYCnMzdf";

BasicDataArea DataArea = {};

void ClearDataArea(ULONG64 start, int type) {
	ULONG64 len;
	switch (type)
	{
	case 0:
		len = 4096;
		break;
	case 1:
		len = 409600;
		break;
	case 2:
		len = 4096000;
		break;
	default:
		return;
	}
	for (ULONG64 i = start; i < len; i++)
	{
		*(BYTE*)i = 0x00;
	}
}

void SetDataArea(PVOID dataArea) {
	DataArea = *(BasicDataArea*)dataArea;
}

NTSTATUS StartThread(PKSTART_ROUTINE start_function,PVOID args) {
	HANDLE thread_handle = nullptr;

	//CONTEXT paco = CONTEXT();

	//paco.Rax = (ULONG64)start_function;

	const NTSTATUS status = PsCreateSystemThread(&thread_handle, GENERIC_ALL, nullptr, nullptr, nullptr, start_function, args);
	if (NT_SUCCESS(status)) {
		ZwClose(thread_handle);
	}
	return status;
}

BOOLEAN GetGlobalActive() {
	return GlobalActive;
}
VOID SetGlobalActive(BOOLEAN st) {
	GlobalActive = st;
}

void Translate(unsigned char* buffer, size_t len) {
	for (size_t i = 0; i < len; i++) {
		for (size_t k = 0; k < 54; k++) {
			if (buffer[i] == modified[k]) {
				buffer[i] = original[k];
			}
		}
	}
}

void Sleep(int ms) {
	LARGE_INTEGER timeout;
	timeout.QuadPart = ms* - 10000;
	KeDelayExecutionThread(KernelMode, FALSE, &timeout);
}

HANDLE FindProcessByName(wchar_t* name, size_t name_len) {

	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);
	J();
	if (!bytes)
	{
		return 0;
	}

	PVOID data = DataArea.ModListDataAddr;//ExAllocatePool(NonPagedPool, bytes); //abrimos espacio para la lista de procesos
	if (data == 0) {
		return 0;
	}

	if (bytes > 409600) {
		log("Error space needed to find process by name\n");
		return NULL;
	}

	status = ZwQuerySystemInformation(SystemProcessInformation, data, bytes, &bytes);
	if (!NT_SUCCESS(status))
	{
		//ExFreePool(data);
		RtlSecureZeroMemory(DataArea.ModListDataAddr, 409600);
		return 0;
	}

	PSYSTEM_PROCESS_INFORMATION pProcess = (PSYSTEM_PROCESS_INFORMATION)data;
	J();
	log("Searching for %ws\n", name);
	while (TRUE)
	{
		if (pProcess == 0) {
			break;
		}
		
		log("Reading %wZ\n", pProcess->ImageName);
		if (&pProcess->ImageName != nullptr && pProcess->ImageName.Length == name_len*2) {
			if (memcmp(pProcess->ImageName.Buffer, name, name_len*2) == 0) {
				log("Process name: %wZ  - Process ID: %d\n", pProcess->ImageName, pProcess->UniqueProcessId);
				HANDLE id = pProcess->UniqueProcessId;
				RtlSecureZeroMemory(DataArea.ModListDataAddr, 409600);
				return id;
			}
			else {
				log("Name missmatch\n");
			}
		}
		else {
			log("Len missmatch %d vs %d\n", pProcess->ImageName.Length, name_len);
		}

		if (pProcess->NextEntryOffset == 0) {
			break;
		}

		pProcess = (PSYSTEM_PROCESS_INFORMATION)((LPBYTE)pProcess + pProcess->NextEntryOffset); // Calculate the address of the next entry.
	}
	RtlSecureZeroMemory(DataArea.ModListDataAddr, 409600);
	return 0;
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

BOOLEAN write(HANDLE pid, ULONG64 addr, ULONG64 data, ULONG64 size) {
	PEPROCESS Process;
	if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &Process)))
		return KeWriteVirtualMemory(Process, (PVOID)data, (PVOID)addr, size) == 0;
	return FALSE;
}

BOOLEAN read(HANDLE pid, ULONG64 addr, ULONG64 dest, ULONG64 size) {
	PEPROCESS Process;
	if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &Process)))
		return KeReadVirtualMemory(Process, (PVOID)addr, (PVOID)dest, size) == 0;
	return FALSE;
}

BOOLEAN fileExists(UNICODE_STRING path) {
	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;
	OBJECT_ATTRIBUTES  objAttr;

	InitializeObjectAttributes(&objAttr, &path,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	ntstatus = ZwOpenFile(&handle, GENERIC_READ, &objAttr, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL);
	if (ntstatus == STATUS_SUCCESS && handle != NULL) {
		ZwClose(handle);
		return TRUE;
	}
	return FALSE;
}