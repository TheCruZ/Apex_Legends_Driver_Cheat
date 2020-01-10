


#include "bypass.h"


PVOID ResolveRelativeAddress(_In_ PVOID Instruction, _In_ ULONG OffsetOffset, _In_ ULONG InstructionSize)
{
	ULONG_PTR Instr = (ULONG_PTR)Instruction;
	LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
	J();
	PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);

	return ResolvedAddr;
}

BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'c' && *pData != *bMask)
			return 0;
	J();
	return (*szMask) == 0;
}

UINT64 FindPattern(UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, char* szMask)
{
	for (UINT64 i = 0; i < dwLen; i++)
		if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
			return (UINT64)(dwAddress + i);
	J();
	return 0;
}

NTOSAddrs* GetNtoskrnlAddress() {
	NTOSAddrs* result = (NTOSAddrs*)DataArea.NTAddr;

	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
	J();
	if (!bytes)
	{
		log("Error Getting System module list size\n");
		return NULL;
	}

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)DataArea.NTModListData;//ExAllocatePool(NonPagedPool, bytes);
	if (modules == NULL) {
		log("Error creating pool for system module list\n");
		return NULL;
	}

	if (bytes > 409600) {
		log("Error space needed to search module ntoskrn too big\n");
		return NULL;
	}

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);
	J();
	if (!NT_SUCCESS(status))
	{
		log("Error reading system module list into his pool\n");
		RtlZeroMemory(DataArea.NTModListData, 409600);
		return NULL;
	}

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	J();
	if (module == NULL) {
		log("Error reading modules from System module list\n");
		RtlZeroMemory(DataArea.NTModListData, 409600);
		return NULL;
	}

	RtlZeroMemory(DataArea.NTName, 4096);
	DataArea.NTName[0] = '\\';
	DataArea.NTName[1] = 'S';
	DataArea.NTName[2] = 'y';
	DataArea.NTName[3] = 's';
	DataArea.NTName[4] = 't';
	DataArea.NTName[5] = 'e';
	DataArea.NTName[6] = 'm';
	DataArea.NTName[7] = 'R';
	DataArea.NTName[8] = 'o';
	DataArea.NTName[9] = 'o';
	DataArea.NTName[10] = 't';
	DataArea.NTName[11] = '\\';
	DataArea.NTName[12] = 's';
	DataArea.NTName[13] = 'y';
	DataArea.NTName[14] = 's';
	DataArea.NTName[15] = 't';
	DataArea.NTName[16] = 'e';
	DataArea.NTName[17] = 'm';
	DataArea.NTName[18] = '3';
	DataArea.NTName[19] = '2';
	DataArea.NTName[20] = '\\';
	DataArea.NTName[21] = 'n';
	DataArea.NTName[22] = 't';
	DataArea.NTName[23] = 'o';
	DataArea.NTName[24] = 's';
	DataArea.NTName[25] = 'k';
	DataArea.NTName[26] = 'r';
	DataArea.NTName[27] = 'n';
	DataArea.NTName[28] = 'l';
	DataArea.NTName[29] = '.';
	DataArea.NTName[30] = 'e';
	DataArea.NTName[31] = 'x';
	DataArea.NTName[32] = 'e';

	uintptr_t ntoskrnlBase = 0, ntoskrnlSize = 0;
	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		if (!strcmp((char*)module[i].FullPathName, DataArea.NTName)) //"\\SystemRoot\\system32\\ntoskrnl.exe"
		{
			ntoskrnlBase = (UINT64)module[i].ImageBase;
			J();
			ntoskrnlSize = (UINT64)module[i].ImageSize;
			break;
		}
	}

	if (ntoskrnlBase == 0) {
		log("BWarning no ntoskrnl found");
		RtlZeroMemory(DataArea.NTName, 50);
		RtlZeroMemory(DataArea.NTModListData, 409600);
		return NULL;
	}

	RtlSecureZeroMemory(DataArea.NTName, 50);
	RtlSecureZeroMemory(DataArea.NTModListData, 409600);

	result->StartAddress = ntoskrnlBase;
	result->Size = ntoskrnlSize;

	return result;
}

uintptr_t SearchNtoskrnlPattern(BYTE* bMask, char* szMask, NTOSAddrs* ntos) {
	if (ntos == NULL) {
		ntos = GetNtoskrnlAddress();
	}
	if (ntos == NULL || ntos->StartAddress == 0 || ntos->Size == 0) {
		log("Can't get Ntoskrnl Address\n");
		return 0;
	}
	J();
	uintptr_t result = (uintptr_t)FindPattern(ntos->StartAddress, ntos->Size, bMask, szMask);

	if (result == 0) {
		log("Warning no Pattern found");
		return 0;
	}

	return result;
}

/*
BOOLEAN CleanUnloadedDrivers()
{
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

	if (!bytes)
	{
		//DbgPrintEx(0, 0, "CleanUnloadedDrivers: first NtQuerySystemInformation failed, status: 0x%x\n", status);
		return FALSE;
	}

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x454E4F45); // 'ENON'

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
	{
		//DbgPrintEx(0, 0, "CleanUnloadedDrivers: second NtQuerySystemInformation failed, status: 0x%x\n", status);
		return FALSE;
	}

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	UINT64 ntoskrnlBase = 0, ntoskrnlSize = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		//DbgPrintEx(0, 0, "CleanUnloadedDrivers: path: %s\n", module[i].FullPathName);

		if (!strcmp((char*)module[i].FullPathName, "\\SystemRoot\\system32\\ntoskrnl.exe"))
		{
			ntoskrnlBase = (UINT64)module[i].ImageBase;
			ntoskrnlSize = (UINT64)module[i].ImageSize;
			break;
		}
	}

	if (modules)
		ExFreePoolWithTag(modules, 0);

	if (ntoskrnlBase <= 0)
	{
		//DbgPrintEx(0, 0, "CleanUnloadedDrivers: ntoskrnlBase equals zero\n");
		return FALSE;
	}

	// NOTE: 4C 8B ? ? ? ? ? 4C 8B C9 4D 85 ? 74 + 3] + current signature address = MmUnloadedDrivers
	UINT64 mmUnloadedDriversPtr = FindPattern((UINT64)ntoskrnlBase, (UINT64)ntoskrnlSize, (BYTE*)"\x4C\x8B\x00\x00\x00\x00\x00\x4C\x8B\xC9\x4D\x85\x00\x74", "xx?????xxxxx?x");

	if (!mmUnloadedDriversPtr)
	{
		//DbgPrintEx(0, 0, "CleanUnloadedDrivers: mmUnloadedDriversPtr equals zero\n");
		return FALSE;
	}

	UINT64 mmUnloadedDrivers = (UINT64)((PUCHAR)mmUnloadedDriversPtr + *(PULONG)((PUCHAR)mmUnloadedDriversPtr + 3) + 7);

	UINT64 mmUnloadedDrivers_bufferPtr = *(UINT64*)mmUnloadedDrivers;

	//MM_UNLOADED_DRIVER* MmUnloadedDrivers = mmUnloadedDrivers_bufferPtr;
	//for (ULONG i = 0; i < 50; i++)
	//{
	//	DbgPrintEx(0,0,"[MmUnloadedDrivers] %d = %wZ\n", i, MmUnloadedDrivers[i].Name);
	//}

	// NOTE: 0x7D0 is the size of the MmUnloadedDrivers array for win 7 and above
	PVOID newBuffer = ExAllocatePoolWithTag(NonPagedPoolNx, 0x7D0, 0x54446D4D);

	if (!newBuffer)
		return FALSE;

	memset(newBuffer, 0, 0x7D0);

	// NOTE: replace MmUnloadedDrivers
	*(UINT64*)mmUnloadedDrivers = (UINT64)newBuffer;
	//DbgPrintEx(0, 0, "mmUnloadedDrivers nulled\n");
	// NOTE: clean the old buffer
	ExFreePoolWithTag((PVOID)mmUnloadedDrivers_bufferPtr, 0x54446D4D); // 'MmDT'

	return TRUE;
}
*/

FORCEINLINE
BOOLEAN
ModRemoveEntryList(
	_In_ PLIST_ENTRY Entry
)

{

	PLIST_ENTRY PrevEntry;
	PLIST_ENTRY NextEntry;

	NextEntry = Entry->Flink;
	PrevEntry = Entry->Blink;

	PrevEntry->Flink = NextEntry;
	NextEntry->Blink = PrevEntry;
	return (BOOLEAN)(PrevEntry->Flink == NextEntry);
}

UINT64 ClearCacheEntry(UNICODE_STRING name) {

	NTOSAddrs* ntos = GetNtoskrnlAddress();
	
	if (ntos == NULL || ntos->StartAddress == 0 || ntos->Size == 0) {
		log("Warning no ntoskrnl found");
		return 0;
	}
	
	
	J();
	PVOID PiDDBLockPtr = (PVOID)SearchNtoskrnlPattern((UCHAR*)"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x4C\x8B\x8C", "ccc!!!!c!!!!ccc", ntos);
	
	J();
	if (PiDDBLockPtr == NULL) {
		log("Warning no PiDDBLockPtr found");
		return 0;
	}
	log("PiDDBLockPtr %llx", PiDDBLockPtr)

	J();
	PVOID PiDDBCacheTablePtr = (PVOID)SearchNtoskrnlPattern((UCHAR*)"\x66\x03\xD2\x48\x8D\x0D", "cccccc", ntos);
	if (PiDDBCacheTablePtr == 0) {
		log("Warning no PiDDBCacheTablePtr found");
		return 0;
	}
	log("PiDDBCacheTablePtr %llx", PiDDBCacheTablePtr)

	// DbgPrintEx(0, 0, "BlockPtr: %p  CacheTable. %p\n", ResolveRelativeAddress(PiDDBLockPtr, 3, 7), ResolveRelativeAddress(PiDDBCacheTablePtr, 6, 10));

	PERESOURCE PiDDBLock; PRTL_AVL_TABLE PiDDBCacheTable;
	J();
	PiDDBLock = (PERESOURCE)ResolveRelativeAddress(PiDDBLockPtr, 3, 7);
	J();
	PiDDBCacheTable = (PRTL_AVL_TABLE)ResolveRelativeAddress(PiDDBCacheTablePtr, 6, 10);

	PiDDBCacheTable->TableContext = (PVOID)1;

	// build a lookup entry
	PiDDBCacheEntry lookupEntry = { 0 };
	J();
	lookupEntry.DriverName = name;
	log("to clear %wZ",lookupEntry.DriverName)
	// until here all is okay (at least not crashes)

	// acquire the ddb resource lock
	if (!ExAcquireResourceExclusiveLite(PiDDBLock, TRUE)) {
		log("Can't acquire resource\n");
		return 0;
	}

	// search our entry in the table
	PiDDBCacheEntry* pFoundEntry = (PiDDBCacheEntry*)RtlLookupElementGenericTableAvl(PiDDBCacheTable, &lookupEntry);
	J();
	if (pFoundEntry == 0)
	{
		log("Not found in cache\n");
		// release the ddb resource lock
		ExReleaseResourceLite(PiDDBLock);

		return 0;
	}
	J();
	log("Found bad driver %wZ\n", pFoundEntry->DriverName);
	// first, unlink from the list
	if (!ModRemoveEntryList(&pFoundEntry->List)) {
		log("Can't unlink from list\n");
		ExReleaseResourceLite(PiDDBLock);
		return 0;
	}
	// then delete the element from the avl table
	if (!RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry)) {
		log("Can't delete from cache\n");
		ExReleaseResourceLite(PiDDBLock);
		return 0;
	}
	J();
	// release the ddb resource lock
	ExReleaseResourceLite(PiDDBLock);

	log("Cleaned %wZ\n", name);

	ClearCacheEntry(name); // if found clear again
	J();
	return 1;
}