#include "bypass.h"
#include "server.h"

#ifdef DEBUG
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	__kernel_entry NTSTATUS NtQueryInformationThread(
		IN HANDLE          ThreadHandle,
		IN THREADINFOCLASS ThreadInformationClass,
		OUT PVOID          ThreadInformation,
		IN ULONG           ThreadInformationLength,
		OUT PULONG         ReturnLength
	);

	NTSTATUS
		PsSetContextThread(
			__in PETHREAD Thread,
			__in PCONTEXT ThreadContext,
			__in KPROCESSOR_MODE Mode
		);

	NTSTATUS 
		PsGetContextThread(
			__in PETHREAD Thread,
			__inout PCONTEXT ThreadContext,
			__in KPROCESSOR_MODE PreviousMode
		);

	NTSYSCALLAPI
		NTSTATUS
		NTAPI
		ZwSuspendThread(
			_In_ HANDLE ThreadHandle,
			_Out_opt_ PULONG PreviousSuspendCount
		);

	
	NTSTATUS PsLookupThreadByThreadId(
		HANDLE   ThreadId,
		PETHREAD* Thread
	);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif

#ifdef DEBUG

//PspCreateThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ProcessHandle, IN PEPROCESS ProcessPointer, OUT PCLIENT_ID ClientId OPTIONAL, IN PCONTEXT ThreadContext OPTIONAL, IN VOID* InitialTeb OPTIONAL, IN BOOLEAN CreateSuspended, IN PKSTART_ROUTINE StartRoutine OPTIONAL, IN PVOID StartContext);
typedef NTSTATUS(* _PspCreateThread)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, PEPROCESS, PCLIENT_ID, PCONTEXT,VOID*, BOOLEAN, PKSTART_ROUTINE, PVOID);
_PspCreateThread PspCreateThread;

typedef NTSTATUS(NTAPI*_PspGetContextThreadInternal)(PETHREAD, PCONTEXT, KPROCESSOR_MODE, KPROCESSOR_MODE, KPROCESSOR_MODE);
_PspGetContextThreadInternal PspGetContextThreadInternal;

typedef NTSTATUS(NTAPI* _PspSetContextThreadInternal)(PETHREAD, PCONTEXT, KPROCESSOR_MODE, KPROCESSOR_MODE, KPROCESSOR_MODE);
_PspSetContextThreadInternal PspSetContextThreadInternal;

typedef struct _ETHREAD
{
	unsigned char shit[0x06A0];
	PVOID Win32StartAddress;
} ETHREAD, * PETHREAD;
#endif

VOID ActiveLoop(_In_ PVOID StartContext) {
	UNICODE_STRING uniName;
	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\s.txt");
	while (GetGlobalActive()) {
		Sleep(2000); //Every 2 seconds check for close driver request
		if (fileExists(uniName)) {
			log("Bye\n");
			SetGlobalActive(FALSE);
			return;
		}
	}
}

void InitThreads() {
	StartThread(ActiveLoop, nullptr);
	StartThread(server_thread, nullptr);
}

#ifdef DEBUG
void TestFunction(void* params) {
	int k = 10000;
	while (k > 0) {
		Sleep(100);
		DbgPrintEx(0, 0, "im the fucking %lu\n", PsGetCurrentThreadId());
		k--;
	}
}


void NewFunction(void* params) {
	DbgPrintEx(0,0,"Thread fucked\n");
}
#endif


VOID RealM(void* args) {
	log("Driver Loading\n");
	Sleep(2000); //make sure kdmapper closed
	log("Driver Loaded\n");
	J();

	//PREPARE BUFFERED AREA
	log("To Allocate %ld bytes\n", sizeof(BasicDataArea));
	//PVOID tmpA = ExAllocatePool(NonPagedPool, sizeof(BasicDataArea));
	//PVOID tmpA = buffer3;
	//if (tmpA == nullptr) {
	//	log("Can't allocate DataArea for buffers\n");
	//	return;
	//}
	////RtlSecureZeroMemory(tmpA, sizeof(BasicDataArea));
	//SetDataArea(tmpA);
	//DataArea = {};

	UNICODE_STRING intelDrv;
	DataArea.IVDName[0] = L'i';
	DataArea.IVDName[2] = L'q';
	DataArea.IVDName[4] = L'v';
	DataArea.IVDName[6] = L'w';
	DataArea.IVDName[8] = L'6';
	DataArea.IVDName[10] = L'4';
	DataArea.IVDName[12] = L'e';
	DataArea.IVDName[14] = L'.';
	DataArea.IVDName[16] = L's';
	DataArea.IVDName[18] = L'y';
	DataArea.IVDName[20] = L's';
	RtlInitUnicodeString(&intelDrv, (PCWSTR)DataArea.IVDName);// L"iqvw64e.sys");
	J();
	const UINT64 result = ClearCacheEntry(intelDrv);

	//clear any trace about the vulnerable driver
	RtlSecureZeroMemory(intelDrv.Buffer, intelDrv.Length);
	intelDrv.Length = 0;
	intelDrv.MaximumLength = 0;
	RtlSecureZeroMemory(DataArea.IVDName, 100);
	if (result == 0) {
		log("Load cancelled, can't clean the PiDDBCache\n");
		return;
	}

	J();

#ifdef DEBUG



	PspGetContextThreadInternal = (_PspGetContextThreadInternal)SearchNtoskrnlPattern((UCHAR*)"\x40\x55\x57\x41\x54\x41\x56\x41\x57\x48\x81\xec\xf0\x01\x00", "ccccccccccccccc", NULL);
	PspSetContextThreadInternal = (_PspSetContextThreadInternal)SearchNtoskrnlPattern((UCHAR*)"\x40\x55\x41\x54\x41\x56\x48\x81\xec\xe0\x01\x00\x00", "ccccccccccccc", NULL);
	PspCreateThread = (_PspCreateThread)SearchNtoskrnlPattern((UCHAR*)"\x40\x55\x53\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\x90\xFE\xFF\xFF", "cccccccccccccccccccc", NULL);
	DbgPrintEx(0, 0, "PspGetContextThreadInternal = %llx\n", PspGetContextThreadInternal);
	DbgPrintEx(0, 0, "PspSetContextThreadInternal = %llx\n", PspSetContextThreadInternal);
	DbgPrintEx(0, 0, "PspCreateThread = %llx\n", PspCreateThread);


	HANDLE thread_handle = nullptr;
	CLIENT_ID xx;

	//NTSTATUS statusx = PsCreateSystemThread(&thread_handle, GENERIC_ALL, nullptr, nullptr, &xx, TestFunction, nullptr);
	//NTSTATUS statusx = PspCreateThread(&thread_handle, GENERIC_ALL, nullptr, nullptr, IoGetCurrentProcess(), nullptr/*&xx*/, nullptr, nullptr, false, TestFunction, nullptr);
	//if (NT_SUCCESS(statusx)) {
	//	DbgPrintEx(0, 0, "created as %d\n", xx.UniqueThread);
	//	PETHREAD thread;
	//	statusx = PsLookupThreadByThreadId(xx.UniqueThread,&thread);
	//	if (NT_SUCCESS(statusx)) {
	//		DbgPrintEx(0, 0, "%d-%llx\n", xx.UniqueThread, (ULONG64)TestFunction);
	//		DbgPrintEx(0, 0, "%llx\n", thread->Win32StartAddress);

	//		thread->Win32StartAddress = (PVOID)0xfffff8057b800000; //Modify StartAddress
	//		statusx = PsLookupThreadByThreadId(xx.UniqueThread, &thread);
	//		if (NT_SUCCESS(statusx)) {
	//			DbgPrintEx(0, 0, "MOD %#010x", thread->Win32StartAddress);
	//		}


	//		ULONG resultx;
	//		statusx= ZwSuspendThread(thread,&resultx);
	//		
	//		CONTEXT Ctx = { 0 };
	//		Ctx.ContextFlags = CONTEXT_ALL;
	//		statusx = -1;
	//		
	//		while (!NT_SUCCESS(statusx)) { //GetContext sometimes fails :S
	//			Sleep(1);
	//			statusx = PspGetContextThreadInternal(thread, &Ctx, KernelMode, 0, 0);
	//		}
	//		
	//		if (NT_SUCCESS(statusx)) {
	//			// Sleep(15000);
	//			DbgPrintEx(0, 0, "%llx %p\n", Ctx.Rip, KeDelayExecutionThread);
	//			Ctx.Rip = (ULONG64)NewFunction;

	//			statusx = -1;
	//			while (!NT_SUCCESS(statusx)) { //GetContext sometimes fails :S
	//				Sleep(1);
	//				statusx = PspSetContextThreadInternal(thread, &Ctx, KernelMode, 0, 0);
	//			}
	//			
	//			DbgPrintEx(0, 0, "OK\n");

	//			//Ctx.Rip=
	//			//context.Rip = (ULONG64)NewFunction;

	//			//statusx=PsSetContextThread(thread, &context, KernelMode);
	//			//if (NT_SUCCESS(statusx)) {
	//			//	//statusx = PsResumeThread(thread,&resultx);
	//			//	DbgPrintEx(0, 0, "Done");
	//			//}
	//			//else {
	//			//	return 4;
	//			//}
	//		}
	//		else {
	//			DbgPrintEx(0, 0, "%llx", statusx);
	//			return 3;
	//		}
	//	}
	//	else {
	//		return 2;
	//	}
	//	
	//	ZwClose(thread_handle);
	//	return 0;
	//}

	//return;

	//PETHREAD X = PsGetCurrentThread();
	//PCONTEXT tcontext;
	//NTSTATUS sta = PsGetContextThread(X, tcontext, KernelMode);

	//if (NT_SUCCESS(sta)) {

	//	tcontext->Header

	//}


	////HANDLE current_thread = PsGetCurrentThread()->Win32StartAddress;
	////DWORD dwStartAddress;
	//NTSTATUS status = 0;//NtQueryInformationThread(current_thread,ThreadQuerySetWin32StartAddress,&dwStartAddress,sizeof(DWORD),NULL);
	//if (NT_SUCCESS(status)) {
	//	PETHREAD X = PsGetCurrentThread();
	//	
	//	ULONG64 x2 = ((ULONG64*)X)[0x0690];
	//	//x2 = x2 + 0x0690;
	//	//PVOID x = *(PVOID*)x2;
	//	log("Start %#010x", x2);
	//}
	//else {
	//	log("NtQueryInformationThread fail %#010x",status);
	//}

#endif
	//INIT
	InitThreads();
	J();
}

extern "C" NTSTATUS MainEntry(PDRIVER_OBJECT, PUNICODE_STRING) {
	StartThread(RealM, nullptr);
	return STATUS_SUCCESS;
}




//uintptr_t pPspGetContextThreadInternal = (uintptr_t)((ULONG*)PsGetContextThread);

//pPspGetContextThreadInternal = pPspGetContextThreadInternal + 6ULL + *(int*)(pPspGetContextThreadInternal + 2); // follow the jmp to PsGetContextThread
//pPspGetContextThreadInternal = *(uintptr_t*)pPspGetContextThreadInternal;

//pPspGetContextThreadInternal += 12;  // go to the call PspGetContextThreadInternal line

//pPspGetContextThreadInternal = pPspGetContextThreadInternal + 5 + *(int*)(pPspGetContextThreadInternal + 1); // Read PspGetContextThreadInternal Addr

//PspGetContextThreadInternal = (_PspGetContextThreadInternal)pPspGetContextThreadInternal;

//DbgPrintEx(0, 0, "PspGetContextThreadInternal = %llx\n", PspGetContextThreadInternal);