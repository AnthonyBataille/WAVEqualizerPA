#include <iostream>
#include "portaudio.h"
#include "devices.hpp"
#include "stream.hpp"
#include "wav.hpp"

/**
 * @brief Open the test audio file and run a simple playback.
 */
void runWAVStream() {
	WAVHandler wH = WAVHandler("C:/Users/batai/Documents/Prog/WAVEqualizerPA/test.wav");
	WAVStream stream(&wH);
	stream.open();
	if (stream.isOpen == false) {
		std::cerr << "Unable to open the stream" << std::endl;
		return;
	}

	stream.start();

	std::cout << "Press Enter to stop." << std::endl;
	std::cin.get();
	
	stream.stop();
	stream.close();
	wH.close();
}

int main(int argc, char* argv[]) {
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError) {
		std::cerr << "PA init error: " << Pa_GetErrorText(err) << std::endl;
		return 1;
	}

	printPADevicesInfo();

	runWAVStream();
	
	Pa_Terminate();
	return 0;
}
