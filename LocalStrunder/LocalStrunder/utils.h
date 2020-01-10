
#pragma once

#include <Windows.h>
//#include <TlHelp32.h>
#include <string>

struct handle_data {
	DWORD process_id;
	HWND window_handle;
};

class Utils {
public:
	//static DWORD GetPID(const std::wstring& procname);
	//static DWORD GetBase(DWORD PID, const std::wstring& modulename);
	static HWND FindMainWindow(DWORD process_id);
};