#include <iostream>
#include "portaudio.h"
#include "devices.hpp"
#include "stream.hpp"

void runStream() {
	SawToothStream defaultStream;

	std::cout << "Press Enter to start." << std::endl;
	std::cin.get();

	defaultStream.start();

	std::cout << "Press Enter to stop." << std::endl;
	std::cin.get();

	defaultStream.stop();
	defaultStream.close();
}

int main(int argc, char* argv[]) {
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError) {
		std::cerr << "PA init error: " << Pa_GetErrorText(err) << std::endl;
		return 1;
	}

	printPADevicesInfo();

	runStream();
	
	Pa_Terminate();
	return 0;
}
