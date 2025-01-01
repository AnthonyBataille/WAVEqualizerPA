#include <iostream>
#include "portaudio.h"
#include "stream.hpp"

void error_handler(const PaError err) {
	std::cout << "error_handler()" << std::endl;
	if (err != paNoError) {
		std::cout << "PA error: " << Pa_GetErrorText(err) << std::endl;
		Pa_Terminate();
		exit(EXIT_FAILURE);
	}
}

/* Stream */

int Stream::callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	(void)inputBuffer;
	(void)outputBuffer;
	(void)framesPerBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;
	return paContinue;
}

PaError Stream::open() {
	isOpen = true;
	return paNoError;
}

PaError Stream::start() {
	PaError err = Pa_StartStream(_stream);
	error_handler(err);
	return err;
}

PaError Stream::stop() {
	PaError err;
	if (Pa_IsStreamActive(_stream) == 1) {
		err = Pa_StopStream(_stream);
		error_handler(err);
	}
	else {
		err = paNoError;
	}
	return err;
}

PaError Stream::close() {
	PaError err;
	if (isOpen) {
		err = Pa_CloseStream(_stream);
		error_handler(err);
		isOpen = false;
	}
	else {
		err = paNoError;
	}
	return err;
}

Stream::Stream() {
	_stream = nullptr;
	isOpen = false;
}

Stream::~Stream() {
	stop();
	close();
}

/* WAVStream */

WAVStream::WAVStream(WAVHandler* wavHandler) {
	wH = wavHandler;
	data.left_phase = 0.0f;
	data.right_phase = 0.0f;
	data.wH = wH;
	_stream = nullptr;
	isOpen = false;
}

int WAVStream::callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	paWavUserData_t* data = (paWavUserData_t*)userData;
	int16_t* out = (int16_t*)outputBuffer;
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	for (unsigned long frameIndex = 0U; frameIndex < framesPerBuffer; ++frameIndex) {
		*out++ = data->left_phase;
		*out++ = data->right_phase;
		data->wH->read_next(&(data->left_phase));
		data->right_phase = 0U;
	}
	
	return paContinue;
}

PaError WAVStream::open() {
	PaError err = Pa_OpenDefaultStream(
		&_stream,
		0,
		2,
		paInt16,
		48000,
		paFramesPerBufferUnspecified,
		&WAVStream::callback,
		&data);

	error_handler(err);

	wH->open();
	if (wH->checkHeader()) {
		if (wH->loadDataChunk()) {
			isOpen = true;
		}
	}
	return err;
}
