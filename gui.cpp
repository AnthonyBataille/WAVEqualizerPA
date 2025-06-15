#include "gui.hpp"

#include <commctrl.h>
#include <commdlg.h>
#include <windows.h>

#include <cwchar>
#include <iostream>

#include "filter.hpp"
#include "portaudio.h"
#include "resource.h"
#include "stream.hpp"
#include "wav.hpp"

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static constexpr wchar_t g_szClassName[] = L"mainWindowClass";

bool errorHandlerPA(const PaError err) {
  if (err != paNoError) {
    const char *PaErrText = Pa_GetErrorText(err);
    int size = MultiByteToWideChar(CP_UTF8, 0, PaErrText, -1, nullptr, 0);
    std::wstring PaErrString(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, PaErrText, -1, PaErrString.data(), size);

    size = std::swprintf(nullptr, 0, L"PA error : %s", PaErrString.data());
    std::wstring err_text(size + 1, '\0');
    std::swprintf(err_text.data(), size + 1, L"PA error : %s",
                  PaErrString.data());
    MessageBox(NULL, err_text.data(), L"Error", MB_ICONERROR | MB_OK);
    return false;
  }
  return true;
}

bool GUI::closeWAVStream() {
  if (audioHandle.stream.isStarted) {
    if (!audioHandle.stream.stop()) {
      MessageBox(_hwnd, L"Unable to stop the stream.", L"Error",
                 MB_ICONERROR | MB_OK);
      return false;
    }

    if (!audioHandle.stream.close()) {
      MessageBox(_hwnd, L"Unable to close the stream.", L"Error",
                 MB_ICONERROR | MB_OK);
      return false;
    }
  }
  return true;
}

/**
 * @brief Open the selected audio file.
 */
bool GUI::openWAVStream(const std::wstring &filePath) {
  // Test values
  constexpr float f_max = 24000.0f;

  audioHandle.wH.init(filePath);
  for (int i = 0; i < NUM_FILTERS; ++i) {
    const float f_c = CENTER_FREQUENCIES[i];
    const float BW = BANDWIDTHS[i];
    const float G = audioHandle.Gains[i];
    audioHandle.filtersLeft[i] = PNFilter(BW, G, f_c, f_max);
    audioHandle.filtersRight[i] = PNFilter(BW, G, f_c, f_max);
  }

  audioHandle.stream = WAVStream(&audioHandle.wH, audioHandle.filtersLeft,
                                 audioHandle.filtersRight);

  if (!audioHandle.stream.open()) {
    return false;
  }
  return true;
}

INT_PTR CALLBACK GUI::AboutDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                   LPARAM lParam) {
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
  } break;
  default:
    return FALSE;
  }
  return TRUE;
}

void GUI::updateFilterGain(HWND hwnd) {
  const int trackbarNum = GetDlgCtrlID(hwnd) - TRACKBAR_ID_PREFIX;
  LRESULT pos = SendMessage(_hTrack[trackbarNum], TBM_GETPOS, 0, 0);
  pos = TRACKBAR_MAXVALUE - pos;
  std::wstring buf(4, L'\0');
  std::swprintf(buf.data(), 4, L"%ld", static_cast<int>(pos));
  SetWindowText(_hlbl[trackbarNum], buf.data());

  const float newGain{
      static_cast<float>(pos - TRACKBAR_MINVALUE) /
      static_cast<float>(TRACKBAR_MAXVALUE - TRACKBAR_MINVALUE) * 2.0f};
  audioHandle.Gains[trackbarNum] = newGain;
  if (audioHandle.stream.isOpen) {
    audioHandle.filtersLeft[trackbarNum].updateGain(newGain);
    audioHandle.filtersRight[trackbarNum].updateGain(newGain);
  }
}

void GUI::createControls(HWND hwnd) {
  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icex);

  for (size_t i = 0U; i < NUM_FILTERS; ++i) {
    HWND &bottomLabel{_hBottomLabel[i]};
    bottomLabel = CreateWindow(L"Static", L"1", WS_CHILD | WS_VISIBLE, 40 * i,
                               0, 10, 20, hwnd, BOT_LABEL_IDS()[i], NULL, NULL);
    if (bottomLabel == NULL) {
      wchar_t error_msg[128];
      DWORD err_code = GetLastError();
      wsprintf(error_msg, L"Left label Failed. Error %d", err_code);
      MessageBox(NULL, error_msg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
    }
  }
  for (size_t i = 0U; i < NUM_FILTERS; ++i) {
    HWND &topLabel{_hTopLabel[i]};
    topLabel = CreateWindow(L"Static", L"100", WS_CHILD | WS_VISIBLE, 40 * i, 0,
                            30, 20, hwnd, TOP_LABEL_IDS()[i], NULL, NULL);
  }

  for (size_t i = 0U; i < NUM_FILTERS; ++i) {
    HWND &label{_hlbl[i]};
    label = CreateWindow(L"Static", L"50", WS_CHILD | WS_VISIBLE, 20 + 40 * i,
                         230, 30, 20, hwnd, CENTER_LABEL_IDS()[i], NULL, NULL);
  }

  for (size_t i = 0U; i < NUM_FILTERS; ++i) {
    HWND &track{_hTrack[i]};
    track = CreateWindow(
        TRACKBAR_CLASS, L"Trackbar Control",
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT | TBS_BOTTOM,
        20 + 40 * i, 30, 20, 170, hwnd, TRACKBAR_LABEL_IDS()[i], NULL, NULL);
    SendMessage(track, TBM_SETRANGE, (WPARAM)TRUE,
                MAKELONG(TRACKBAR_MINVALUE, TRACKBAR_MAXVALUE));
    SendMessage(track, TBM_SETPAGESIZE, 0, 10);
    SendMessage(track, TBM_SETTICFREQ, 10, 0);
    SendMessage(track, TBM_SETPOS, FALSE,
                (TRACKBAR_MINVALUE + TRACKBAR_MAXVALUE) / 2);
    SendMessage(track, TBM_SETBUDDY, (WPARAM)FALSE, (LPARAM)_hBottomLabel[i]);
    SendMessage(track, TBM_SETBUDDY, (WPARAM)TRUE, (LPARAM)_hTopLabel[i]);
  }

  constexpr int vertLinePos{20 + 40 * (NUM_FILTERS - 1) + 40};
  HWND verticalSeparator = CreateWindow(
      L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, vertLinePos, 0, 1,
      320, hwnd, (HMENU)VERT_SEP_ID, NULL, NULL);

  _wavFileName.reserve(MAX_PATH);
  ZeroMemory(&_wavFileNameDialog, sizeof(_wavFileNameDialog));
  _wavFileNameDialog.lStructSize = sizeof(_wavFileNameDialog);
  _wavFileNameDialog.hwndOwner = hwnd;
  _wavFileNameDialog.lpstrFilter =
      L"WAV Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
  _wavFileNameDialog.lpstrFile = _wavFileName.data();
  _wavFileNameDialog.nMaxFile = MAX_PATH;
  _wavFileNameDialog.Flags =
      OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  _wavFileNameDialog.lpstrDefExt = L"wav";

  HWND fileNameLabel = CreateWindow(
      L"Static", L"WAV file path:", WS_CHILD | WS_VISIBLE, vertLinePos + 10, 20,
      100, 20, hwnd, (HMENU)FILE_NAME_LABEL_ID, NULL, NULL);

  _fileNameTextBox = CreateWindow(L"Edit", L"",
                                  WS_CHILD | WS_VISIBLE | WS_BORDER |
                                      ES_READONLY | ES_AUTOHSCROLL,
                                  vertLinePos + 10, 50, 170, 20, hwnd,
                                  (HMENU)FILE_NAME_TEXTBOX_ID, NULL, NULL);

  HWND openFileButton = CreateWindow(
      L"Button", L"Open File",
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, vertLinePos + 10,
      80, 100, 20, hwnd, (HMENU)OPEN_FILE_BUTTON_ID, NULL, NULL);

  constexpr int horzLinePos{120};
  HWND horizontalSeparator = CreateWindow(
      L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, vertLinePos,
      horzLinePos, 200, 1, hwnd, (HMENU)HORZ_SEP_ID, NULL, NULL);

  HWND playButton = CreateWindow(
      L"Button", L"Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
      vertLinePos + 20, horzLinePos + 30, 70, 20, hwnd, (HMENU)PLAY_BUTTON_ID,
      NULL, NULL);

  HWND stopButton = CreateWindow(
      L"Button", L"Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
      vertLinePos + 100, horzLinePos + 30, 70, 20, hwnd, (HMENU)STOP_BUTTON_ID,
      NULL, NULL);
}

LRESULT CALLBACK GUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                              LPARAM lParam) {
  switch (msg) {
  case WM_NCCREATE: {
    CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
    SetLastError(0);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
    DWORD err_code = GetLastError();
    if (err_code != 0) {
      wchar_t err_msg[128];
      wsprintf(err_msg, L"Window NC create Failed. Error %d", err_code);
      MessageBox(NULL, err_msg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
      return FALSE;
    }
    return TRUE;
  } break;
  case WM_CREATE: {
    GUI *pgui = (GUI *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    pgui->createControls(hwnd);
  } break;
  case WM_COMMAND: {
    GUI *pgui = (GUI *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (LOWORD(wParam)) {
    case ID_FILE_EXIT: {
      PostMessage(hwnd, WM_CLOSE, 0, 0);
    } break;
    case ID_HELP_ABOUT: {
      DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd,
                &GUI::AboutDlgProc);
    } break;
    case OPEN_FILE_BUTTON_ID: {
      if (GetOpenFileName(&(pgui->_wavFileNameDialog))) {
        SetWindowText(pgui->_fileNameTextBox, pgui->_wavFileName.c_str());
        if (!pgui->closeWAVStream()) {
          MessageBox(hwnd, L"Unable to close the previous stream.", L"Error",
                     MB_ICONERROR | MB_OK);
        }
        if (!pgui->openWAVStream(pgui->_wavFileName)) {
          MessageBox(hwnd, L"Unable to open the stream.", L"Error",
                     MB_ICONERROR | MB_OK);
        }
      } else {
        wchar_t error_msg[128];
        DWORD err_code = GetLastError();
        if (err_code != 0U) {
          wsprintf(error_msg, L"Open file error %d", err_code);
          MessageBox(hwnd, error_msg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        }
      }
    } break;
    case PLAY_BUTTON_ID: {
      if (!pgui->audioHandle.stream.isStarted) {
        if (!pgui->audioHandle.stream.start()) {
          std::cerr << "Unable to start the stream." << std::endl;
          MessageBox(hwnd, L"Unable to start the stream.", L"Error",
                     MB_ICONERROR | MB_OK);
        }
      }
    } break;
    case STOP_BUTTON_ID: {
      if (pgui->audioHandle.stream.isStarted) {
        if (!pgui->audioHandle.stream.stop()) {
          std::cerr << "Unable to stop the stream." << std::endl;
          MessageBox(hwnd, L"Unable to stop the stream.", L"Error",
                     MB_ICONERROR | MB_OK);
        }
      }
    }
    }
  } break;
  case WM_VSCROLL: {
    GUI *pgui = (GUI *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    pgui->updateFilterGain((HWND)lParam);
  } break;
  case WM_CLOSE: {
    GUI *pgui = (GUI *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!pgui->closeWAVStream()) {
      MessageBox(hwnd, L"Unable to close the stream.", L"Error",
                 MB_ICONERROR | MB_OK);
    }
    DestroyWindow(hwnd);
  } break;
  case WM_DESTROY: {
    PostQuitMessage(0);
  } break;
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
  _wc.hIconSm =
      (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON),
                       IMAGE_ICON, 16, 16, 0);
  if (!RegisterClassEx(&_wc)) {
    wchar_t error_msg[128];
    DWORD err_code = GetLastError();
    wsprintf(error_msg, L"Window Registration Failed. Error %d", err_code);
    MessageBox(NULL, error_msg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }

  _hwnd = CreateWindowEx(
      WS_EX_CLIENTEDGE, g_szClassName, L"WAVEqualizerPA",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
      CW_USEDEFAULT, 20 + 40 * (NUM_FILTERS - 1) + 40 + 220, 320, NULL, NULL,
      _hInstance, this);
  if (_hwnd == NULL) {
    wchar_t error_msg[128];
    DWORD err_code = GetLastError();
    wsprintf(error_msg, L"Window Creation Failed. Error %d", err_code);
    MessageBox(NULL, error_msg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  return true;
}

void GUI::runMessageLoop(MSG &msg) {
  ShowWindow(_hwnd, _nCmdShow);
  UpdateWindow(_hwnd);

  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

GUI::GUI(HINSTANCE hInstance, const int nCmdShow, bool &result)
    : _wc(), _hwnd(NULL), _hInstance(NULL), _nCmdShow(0), _hlbl(), _hTrack(),
      _hBottomLabel(), _hTopLabel(), _wavFileNameDialog(), _wavFileName(),
      _fileNameTextBox(NULL), audioHandle() {
  _hInstance = hInstance;
  _nCmdShow = nCmdShow;
  _wavFileName.resize(MAX_PATH);
  result = registerWindowClass();
}
