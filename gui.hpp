#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <array>
#include <string>
#include "stream.hpp"

/* List of GUI objects identifiers */

constexpr int TRACKBAR_MINVALUE = 0;
constexpr int TRACKBAR_MAXVALUE = 100;
constexpr int BOT_LABEL_ID_PREFIX = 300;
constexpr int TOP_LABEL_ID_PREFIX = 400;
constexpr int CENTER_LABEL_ID_PREFIX = 500;
constexpr int TRACKBAR_ID_PREFIX = 600;
constexpr int VERT_SEP_ID = 701;
constexpr int HORZ_SEP_ID = 702;
constexpr int OPEN_FILE_DIALOG_ID = 801;
constexpr int FILE_NAME_LABEL_ID = 802;
constexpr int FILE_NAME_TEXTBOX_ID = 803;
constexpr int OPEN_FILE_BUTTON_ID = 804;
constexpr int PLAY_BUTTON_ID = 805;
constexpr int STOP_BUTTON_ID = 806;

constexpr std::array<HMENU, NUM_FILTERS> BOT_LABEL_IDS() {
	std::array<HMENU, NUM_FILTERS> res{};
	for (int i = 0; i < NUM_FILTERS; ++i) {
		res[i] = (HMENU)(BOT_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_FILTERS> TOP_LABEL_IDS() {
	std::array<HMENU, NUM_FILTERS> res{};
	for (int i = 0; i < NUM_FILTERS; ++i) {
		res[i] = (HMENU)(TOP_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_FILTERS> CENTER_LABEL_IDS() {
	std::array<HMENU, NUM_FILTERS> res{};
	for (int i = 0; i < NUM_FILTERS; ++i) {
		res[i] = (HMENU)(CENTER_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_FILTERS> TRACKBAR_LABEL_IDS() {
	std::array<HMENU, NUM_FILTERS> res{};
	for (int i = 0; i < NUM_FILTERS; ++i) {
		res[i] = (HMENU)(TRACKBAR_ID_PREFIX + i);
	}
	return res;
}

/**
 * @brief The GUI class implements the graphical user interface for opening/closing audio files, playing/stopping player,
 * add the equalizer trackbars (8 in total).
 */
class GUI {
private:
	WNDCLASSEX _wc;
	HWND _hwnd;
	HINSTANCE _hInstance;
	int _nCmdShow;

	std::array<HWND, 8> _hlbl;
	std::array<HWND, 8> _hTrack;
	std::array<HWND, 8> _hBottomLabel;
	std::array<HWND, 8> _hTopLabel;
	OPENFILENAME _wavFileNameDialog;
	std::wstring _wavFileName;
	HWND _fileNameTextBox;

	bool openWAVStream(const std::wstring& filePath);
	bool closeWAVStream();

	void createControls(HWND hwnd);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void updateFilterGain(HWND hwnd);
	bool registerWindowClass();

	

public:
	AudioHandle audioHandle;

	void runMessageLoop(MSG& msg);

	GUI::GUI(HINSTANCE hInstance, const int nCmdShow, bool& result);
	GUI::~GUI() = default;
};
