#include <iostream>
#include "portaudio.h"
#include "gui.hpp"

#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow) {
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError) {
		std::cerr << "PA init error: " << Pa_GetErrorText(err) << std::endl;
		return 1;
	}
	MSG msg{};
	msg.wParam = 0U;
	msg.lParam = 0U;
	{
		bool guiInitSuccess = false;
		GUI mainGUI(hInstance, nCmdShow, guiInitSuccess);
		if (guiInitSuccess) {
			mainGUI.runMessageLoop(msg);
		}
	}
	err = Pa_Terminate();
	
	return static_cast<int>(msg.wParam);
}
