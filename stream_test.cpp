#include <iostream>
#include "portaudio.h"
#include "stream_test.hpp"

/* SawToothStream */

int SawToothStream::callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	paTestData_t* data = (paTestData_t*)userData;
	float* out = (float*)outputBuffer;
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	for (unsigned long i = 0; i < framesPerBuffer; ++i) {
		*out++ = data->left_phase;
		*out++ = data->right_phase;
		data->left_phase += 0.001f;
		if (data->left_phase >= 1.0f) {
			data->left_phase = -1.0f;
		}

		data->right_phase += 0.001f;
		if (data->right_phase >= 2.0f) {
			data->right_phase = -1.0f;
		}
	}

	return paContinue;
}

PaError SawToothStream::open() {
	PaError err = Pa_OpenDefaultStream(
		&_stream,
		0,
		2,
		paFloat32,
		48000,
		paFramesPerBufferUnspecified,
		&SawToothStream::callback,
		&data);

	error_handler(err);
	isOpen = true;
	return err;
}

SawToothStream::SawToothStream() {
	data.left_phase = 0.0f;
	data.right_phase = 0.0f;
	_stream = nullptr;
	isOpen = false;
}

void runTestStream() {
	SawToothStream defaultStream;
	defaultStream.open();
	if (defaultStream.isOpen == false) {
		std::cerr << "Unable to open the stream" << std::endl;
		return;
	}

	std::cout << "Press Enter to start." << std::endl;
	std::cin.get();

	defaultStream.start();

	std::cout << "Press Enter to stop." << std::endl;
	std::cin.get();

	defaultStream.stop();
	defaultStream.close();
}
