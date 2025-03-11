#include <iostream>
#include "portaudio.h"
#include "devices.hpp"
#include "stream.hpp"
#include "wav.hpp"
#include "filter.hpp"
#include "gui.hpp"

#include <windows.h>

/**
 * @brief Open the test audio file and run a simple playback.
 */
void runWAVStream() {
	WAVHandler wH = WAVHandler("C:/Users/batai/Documents/Prog/WAVEqualizerPA/The Top of the World ~Dance Mix~.wav");
	const float f_c = 6000.0f;
	const float _BW = 500.0f;
	const float f_max = 24000.0f;
	const float _G = 0.01f;
	PNFilter filter_left(_BW, _G, f_c, f_max);
	PNFilter filter_right(_BW, _G, f_c, f_max);
	WAVStream stream(&wH, &filter_left, &filter_right);
	;
	if (!stream.open()) {
		std::cerr << "Unable to open the stream." << std::endl;
		return;
	}

	std::cout << "Press Enter to start." << std::endl;
	std::cin.get();

	if (!stream.start()) {
		std::cerr << "Unable to start the stream." << std::endl;
		return;
	}

	std::cout << "Press Enter to stop." << std::endl;
	std::cin.get();
	
	if (!stream.stop()) {
		std::cerr << "Unable to stop the stream." << std::endl;
		return;
	}

	if (!stream.close()) {
		std::cerr << "Unable to close the stream." << std::endl;
		return;
	}
	wH.close();
}

//int main(int argc, char* argv[]) {
//	(void)argc;
//	(void)argv;
//	PaError err;
//	err = Pa_Initialize();
//	if (err != paNoError) {
//		std::cerr << "PA init error: " << Pa_GetErrorText(err) << std::endl;
//		return 1;
//	}
//
//	printPADevicesInfo();
//
//	runWAVStream();
//	
//	Pa_Terminate();
//	return 0;
//}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow) {
	MSG msg{};
	bool guiInitSuccess = false;
	GUI mainGUI(hInstance, nCmdShow, guiInitSuccess);
	if (guiInitSuccess) {
		mainGUI.runMessageLoop(msg);
	}
	
	return static_cast<int>(msg.wParam);
}
