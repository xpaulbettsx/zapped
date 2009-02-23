// Zapped.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Zapped.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE ghInst;								// current instance

/* I hate C++ so bad
const wchar_t* ProcessNameWhitelist[] = {
	"dwm.exe",
	"devenv.exe",
	"zapped.exe",
	NULL
};
*/

// Forward Declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

bool IsProcessNameInWhitelist(const wchar_t* ProcessName)
{
	// C++ sucks so bad
	if (!wcscmp(ProcessName, L"dwm.exe"))
		return true;
	if (!wcscmp(ProcessName, L"devenv.exe"))
		return true;
	if (!wcscmp(ProcessName, L"Zapped.exe"))
		return true;

	return false;
}

void ExecuteZap(void)
{
	// Grab the current user's SID
	PSID thisSid;
	if (GetSecurityInfo(GetCurrentProcess(), SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION, &thisSid, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
		return;

	PWTS_PROCESS_INFO proc_info = NULL;
	DWORD count = 0;
	if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &proc_info, &count)) {
		DWORD gle = GetLastError();
		return;
	}

	for(int i=0; i < count; i++) {
		if (!proc_info[i].pUserSid || !EqualSid(thisSid, proc_info[i].pUserSid))
			continue;
		if (IsProcessNameInWhitelist(proc_info[i].pProcessName))
			continue;
		OutputDebugString(proc_info[i].pProcessName);
		OutputDebugString(L"\n");
	}

	WTSFreeMemory(proc_info);
	ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 
				  SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION | SHTDN_REASON_FLAG_PLANNED);
}

static bool gfHasBeenRegistered = false;
HWND CreateScratchWindow(HWND hwndParent, WNDPROC wp)
{
	if (!gfHasBeenRegistered) {
		WNDCLASS wc = {
			0, DefWindowProc, 0, 0, ghInst, NULL,
			LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE+1),
			NULL, TEXT("ScratchWindow"),
		};
		RegisterClass(&wc);
		gfHasBeenRegistered = true;
	}

	HWND ret;
	ret = CreateWindow(TEXT("ScratchWindow"), NULL,
			(hwndParent ? WS_CHILD : WS_OVERLAPPED),
			0, 0, 0, 0, hwndParent, NULL, NULL, NULL);
	return ret;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	MSG msg;
	ghInst = hInstance;

	// Create a hidden window that can retrieve our notifications
	HWND hwnd = CreateScratchWindow(NULL, WndProc);
	
	// Register our global hotkey
	RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, VK_BACK);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_HOTKEY:
		ExecuteZap();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
