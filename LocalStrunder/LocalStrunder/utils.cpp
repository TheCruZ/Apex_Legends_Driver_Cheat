#include "utils.h"

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}
HWND  Utils::FindMainWindow(DWORD process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

//DWORD Utils::GetBase(DWORD PID, const std::wstring& modulename)
//{
//	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
//	DWORD dwModuleBaseAddress = 0;
//	if (hSnapshot != INVALID_HANDLE_VALUE)
//	{
//		MODULEENTRY32 ModuleEntry32 = { 0 };
//		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
//		if (Module32First(hSnapshot, &ModuleEntry32))
//		{
//			do
//			{
//				if (!modulename.compare(ModuleEntry32.szModule))
//				{
//					dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
//					break;
//				}
//			} while (Module32Next(hSnapshot, &ModuleEntry32));
//		}
//		CloseHandle(hSnapshot);
//	}
//	return dwModuleBaseAddress;
//}

//DWORD Utils::GetPID(const std::wstring& procname)
//{
//	PROCESSENTRY32 processInfo;
//	processInfo.dwSize = sizeof(processInfo);
//
//	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
//	if (processesSnapshot == INVALID_HANDLE_VALUE) {
//		return 0;
//	}
//
//	Process32First(processesSnapshot, &processInfo);
//	if (!procname.compare(processInfo.szExeFile))
//	{
//		CloseHandle(processesSnapshot);
//		return processInfo.th32ProcessID;
//	}
//
//	while (Process32Next(processesSnapshot, &processInfo))
//	{
//		if (!procname.compare(processInfo.szExeFile))
//		{
//			CloseHandle(processesSnapshot);
//			return processInfo.th32ProcessID;
//		}
//	}
//
//	CloseHandle(processesSnapshot);
//	return 0;
//}