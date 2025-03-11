#include "gui.hpp"
#include "resource.h"

#include <windows.h>
#include <commctrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static constexpr wchar_t g_szClassName[] = L"mainWindowClass";


INT_PTR CALLBACK GUI::AboutDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
	break;
	}
	break;
	default:
		return FALSE;
	}
	return TRUE;
}

void GUI::updateLabel(HWND hwnd) {
	int trackbarNum = GetDlgCtrlID(hwnd) - TRACKBAR_ID_PREFIX;
	//int centerLabelID = trackbarNum + CENTER_LABEL_ID_PREFIX;
	LRESULT pos = SendMessage(_hTrack[trackbarNum], TBM_GETPOS, 0, 0);
	pos = TRACKBAR_MAXVALUE - pos;
	wchar_t buf[4];
	wsprintf(buf, L"%ld", pos);
	SetWindowText(_hlbl[trackbarNum], buf);
}

void GUI::createControls(HWND hwnd) {
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	for (size_t i = 0U; i < NUM_TRACKBARS; ++i) {
		HWND& leftLabel{ _hBottomLabel[i] };
		leftLabel = CreateWindow(L"Static", L"0", WS_CHILD | WS_VISIBLE, 40*i, 0, 10, 20, hwnd,
			BOT_LABEL_IDS()[i], NULL, NULL);
		if (leftLabel == NULL) {
			wchar_t error_msg[128];
			DWORD err_code = GetLastError();
			wsprintf(error_msg, L"Left label Failed. Error %d", err_code);
			MessageBox(NULL, error_msg, L"Error!",
				MB_ICONEXCLAMATION | MB_OK);
		}
	}
	for (size_t i = 0U; i < NUM_TRACKBARS; ++i) {
		HWND& rightLabel{ _hTopLabel[i] };
		rightLabel = CreateWindow(L"Static", L"100", WS_CHILD | WS_VISIBLE, 40*i, 0, 30, 20, hwnd,
			TOP_LABEL_IDS()[i], NULL, NULL);
	}
	
	for (size_t i = 0U; i < NUM_TRACKBARS; ++i) {
		HWND& label{ _hlbl[i] };
		label = CreateWindow(L"Static", L"0", WS_CHILD | WS_VISIBLE, 20+40*i, 230, 30, 30, hwnd,
			CENTER_LABEL_IDS()[i], NULL, NULL);
	}
	
	for (size_t i = 0U; i < NUM_TRACKBARS; ++i) {
		HWND& track{ _hTrack[i] };
		track = CreateWindow(TRACKBAR_CLASS, L"Trackbar Control", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT | TBS_BOTTOM,
			20+40*i, 30, 20, 170, hwnd, TRACKBAR_LABEL_IDS()[i], NULL, NULL);
		SendMessage(track, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(TRACKBAR_MINVALUE, TRACKBAR_MAXVALUE));
		SendMessage(track, TBM_SETPAGESIZE, 0, 10);
		SendMessage(track, TBM_SETTICFREQ, 10, 0);
		SendMessage(track, TBM_SETPOS, FALSE, 0);
		SendMessage(track, TBM_SETBUDDY, (WPARAM)FALSE, (LPARAM)_hBottomLabel[i]);
		SendMessage(track, TBM_SETBUDDY, (WPARAM)TRUE, (LPARAM)_hTopLabel[i]);
	}
}

LRESULT CALLBACK GUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_NCCREATE: {
		CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
		SetLastError(0);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
		DWORD err_code = GetLastError();
		if (err_code != 0) {
			wchar_t err_msg[128];
			wsprintf(err_msg, L"Window NC create Failed. Error %d", err_code);
			MessageBox(NULL, err_msg, L"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
		return TRUE;
	}
	break;
	case WM_CREATE: {
		GUI* pgui = (GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		pgui->createControls(hwnd);
	}
	break;
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case ID_FILE_EXIT: {
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
		case ID_HELP_ABOUT: {
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd, &GUI::AboutDlgProc);
		}
		break;
		}
	}
	break;
	case WM_VSCROLL: {
		GUI* pgui = (GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		pgui->updateLabel((HWND)lParam);
	}
	break;
	case WM_CLOSE: {
		DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

bool GUI::registerWindowClass() {
	_wc.cbSize = sizeof(WNDCLASSEX);
	_wc.style = 0;
	_wc.lpfnWndProc = &GUI::WndProc;
	_wc.cbClsExtra = 0;
	_wc.cbWndExtra = 0;
	_wc.hInstance = _hInstance;
	_wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
	_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	_wc.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);
	_wc.lpszClassName = g_szClassName;
	_wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);
	if (!RegisterClassEx(&_wc)) {
		wchar_t error_msg[128];
		DWORD err_code = GetLastError();
		wsprintf(error_msg, L"Window Registration Failed. Error %d", err_code);
		MessageBox(NULL, error_msg, L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	_hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		L"WAVEqualizerPA",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 460, 320,
		NULL, NULL, _hInstance, this);
	if (_hwnd == NULL) {
		wchar_t error_msg[128];
		DWORD err_code = GetLastError();
		wsprintf(error_msg, L"Window Creation Failed. Error %d", err_code);
		MessageBox(NULL, error_msg, L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	return true;
}

void GUI::runMessageLoop(MSG& msg) {
	ShowWindow(_hwnd, _nCmdShow);
	UpdateWindow(_hwnd);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

GUI::GUI(HINSTANCE hInstance, int nCmdShow, bool& result) {
	_hInstance = hInstance;
	_nCmdShow = nCmdShow;
	result = registerWindowClass();
}
