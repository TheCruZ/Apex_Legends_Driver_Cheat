#pragma once


#include <Windows.h>
#include <WinUser.h>
#include "DirectX.h"
#include <Dwmapi.h> 
#pragma comment(lib, "dwmapi.lib")
#include <stdlib.h>
#include <vector>
#include <chrono>
#include <cwchar>

typedef void (*RenderCallback) (void* ov);

struct Process_Informations {
	HWND targetHWND;
	HWND overlayHWND;
	LPCWSTR className;
	MSG msg;
	RECT wRect;
};

class Overlay {
public:
	void Start();
	DWORD CreateOverlay();
	void Render();
	void Clear();
	void SetRender(RenderCallback render);
	bool isRunning();
	void SetTargetHWND(HWND hwnd);
	void UpdatePosition();
	int getWidth();
	int getHeight();
	Direct CurrentDirectX;
private:
	bool running;
	RenderCallback RenderCallbacks;
	Process_Informations proc;
};

