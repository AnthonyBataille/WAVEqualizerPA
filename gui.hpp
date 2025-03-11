#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <array>

constexpr int NUM_TRACKBARS = 8;
constexpr int TRACKBAR_MINVALUE = 0;
constexpr int TRACKBAR_MAXVALUE = 100;
constexpr int BOT_LABEL_ID_PREFIX = 300;
constexpr int TOP_LABEL_ID_PREFIX = 400;
constexpr int CENTER_LABEL_ID_PREFIX = 500;
constexpr int TRACKBAR_ID_PREFIX = 600;

constexpr std::array<HMENU, NUM_TRACKBARS> BOT_LABEL_IDS() {
	std::array<HMENU, NUM_TRACKBARS> res{};
	for (int i = 0; i < NUM_TRACKBARS; ++i) {
		res[i] = (HMENU)(BOT_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_TRACKBARS> TOP_LABEL_IDS() {
	std::array<HMENU, NUM_TRACKBARS> res{};
	for (int i = 0; i < NUM_TRACKBARS; ++i) {
		res[i] = (HMENU)(TOP_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_TRACKBARS> CENTER_LABEL_IDS() {
	std::array<HMENU, NUM_TRACKBARS> res{};
	for (int i = 0; i < NUM_TRACKBARS; ++i) {
		res[i] = (HMENU)(CENTER_LABEL_ID_PREFIX + i);
	}
	return res;
}

constexpr std::array<HMENU, NUM_TRACKBARS> TRACKBAR_LABEL_IDS() {
	std::array<HMENU, NUM_TRACKBARS> res{};
	for (int i = 0; i < NUM_TRACKBARS; ++i) {
		res[i] = (HMENU)(TRACKBAR_ID_PREFIX + i);
	}
	return res;
}

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

	void createControls(HWND hwnd);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void updateLabel(HWND hwnd);
	bool registerWindowClass();

public:
	void runMessageLoop(MSG& msg);

	GUI::GUI(HINSTANCE hInstance, int nCmdShow, bool& result);
	GUI::~GUI() = default;
};
