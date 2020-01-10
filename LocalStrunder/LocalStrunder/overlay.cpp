#include "overlay.h"

bool moving = false;

static DWORD WINAPI StaticPosStart(void* Param)
{
	Overlay* ov = (Overlay*)Param;
	ov->UpdatePosition();
	return 0;
}

static DWORD WINAPI StaticMessageStart(void* Param)
{
	Overlay* ov = (Overlay*)Param;
	ov->CreateOverlay();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(1);
		return 0;

	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return 0;
}

unsigned long getmsx() {
	namespace sc = std::chrono;
	sc::milliseconds ms = sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch());
	return ms.count();
}

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
	wchar_t className[255] = L"";
	GetClassName(hwnd, className, 255);
	if (wcscmp(L"LocalStrunder", className) == 0) {
		Process_Informations* proc = (Process_Informations*)lParam;
		proc->overlayHWND = hwnd;
		return TRUE;
	}
	return TRUE;
}

DWORD Overlay::CreateOverlay()
{
	EnumWindows(EnumWindowsCallback, (LPARAM)&proc);
	Sleep(300);
	if (proc.overlayHWND == 0) {
		printf("ReNotFound\n");
		Sleep(1000);
		exit(0);
	}

	if (!Direct::D3DInit(proc.overlayHWND, CurrentDirectX)) {
		//CloseWindow(proc.overlayHWND);
		//UnregisterClass(proc.className, NULL);
		return 1;
	}

	if (CurrentDirectX.dx_Device == 0 ||
		CurrentDirectX.dx_Object == 0) {
		CurrentDirectX.D3DShutdown();
		//CloseWindow(proc.overlayHWND);
		//UnregisterClass(proc.className, NULL);
		return 1;
	}
	running = 1;
	DWORD ThreadID;
	CreateThread(NULL, 0, StaticPosStart, (void*)this, 0, &ThreadID);
	int rs = 255;
	int gs = 0;
	int bs = 0;
	unsigned long msx = getmsx();
	LPCWSTR title = L"Local Strunder";
	int wtime = 1000 / 20;  // fps rate limit
	while (running) {
		
		//if (PeekMessage(&proc.msg, proc.overlayHWND, NULL, NULL, PM_REMOVE))
		//{
		//	TranslateMessage(&proc.msg);
		//	DispatchMessage(&proc.msg);
		//}
		if (moving) {
			Sleep(25);
			continue;
		}
		unsigned long ms = getmsx();
		if (CurrentDirectX.dx_Device != 0) {
			try {
				CurrentDirectX.dx_Device->Clear(NULL, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, NULL);
				CurrentDirectX.dx_Device->BeginScene();
				CurrentDirectX.DrawString(10, 10, 255, rs, gs, bs, title);
				Render();
				CurrentDirectX.dx_Device->EndScene();
				CurrentDirectX.dx_Device->Present(NULL, NULL, NULL, NULL);
			}
			catch (...) {
			}
		}
		unsigned long ms2 = getmsx()-ms;
		if (ms2 < wtime) {
			Sleep(wtime - ms2);
		}
		if (ms>msx+250) {
			msx = getmsx();
			if (rs == 255) {
				rs = 0;
				gs = 255;
			}
			else if (gs == 255) {
				gs = 0;
				bs = 255;
			}
			else {
				bs = 0;
				rs = 255;
			}
		}

	}
	return 0;
}

void Overlay::UpdatePosition() {
	RECT rect;
	while (running) {
		Sleep(2000);
		SetLastError(0);
		GetWindowRect(proc.targetHWND, &rect);
		if (GetLastError() == 0x00000578) { //INVALID HANDLE (Target Window closed)
			//CloseWindow(proc.overlayHWND);
			//UnregisterClass(proc.className, NULL);
			//CurrentDirectX.D3DShutdown();
			//running = FALSE;
			//return 1;
			continue;
		}
		
		if (proc.wRect.left != rect.left ||
			proc.wRect.top != rect.top ||
			proc.wRect.bottom != rect.bottom ||
			proc.wRect.right != rect.right)
		{
			moving = true;
			Sleep(100);
			if (!SetWindowPos(proc.overlayHWND, HWND_TOPMOST, rect.left, rect.top, rect.right, rect.bottom, SWP_NOACTIVATE)) {
				this->Clear();
				return;
			}
			CurrentDirectX.D3DShutdown();
			if (!Direct::D3DInit(proc.overlayHWND, CurrentDirectX)) {
				//CloseWindow(proc.overlayHWND);
				//UnregisterClass(proc.className, NULL);
				running = 0;
				return;
			}
			proc.wRect = rect;
			Sleep(100);
			moving = false;
		}
	}
}

void Overlay::SetTargetHWND(HWND hwnd) {
	proc.targetHWND = hwnd;
}

void Overlay::Start()
{
	running = 1;
	DWORD ThreadID;
	
	CreateThread(NULL, 0, StaticMessageStart, (void*)this, 0, &ThreadID);
}

void Overlay::Render() {
	(*(RenderCallbacks))(this);
}

void Overlay::SetRender(RenderCallback v) {
	RenderCallbacks = v;
}

bool Overlay::isRunning() {
	return running;
}

void Overlay::Clear() {
	running = FALSE;
	CurrentDirectX.D3DShutdown();
	//CloseWindow(proc.overlayHWND);
	//UnregisterClass(proc.className, NULL);
}

int Overlay::getWidth() {
	return proc.wRect.right - proc.wRect.left;
}
int Overlay::getHeight() {
	return proc.wRect.bottom - proc.wRect.top;
}