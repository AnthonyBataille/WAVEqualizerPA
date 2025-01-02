#include <iostream>
#include "portaudio.h"
#include "stream.hpp"

bool error_handler(const PaError err) {
	if (err != paNoError) {
		std::cout << "PA error: " << Pa_GetErrorText(err) << std::endl;
		Pa_Terminate();
		return false;
	}
	return true;
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

bool Stream::open() {
	isOpen = true;
	return true;
}

bool Stream::start() {
	PaError err = Pa_StartStream(_stream);
	error_handler(err);
	return (err == paNoError);
}

bool Stream::stop() {
	PaError err = Pa_IsStreamActive(_stream);
	if (err == 1) {
		err = Pa_StopStream(_stream);
		error_handler(err);
	}
	return (err == paNoError);
}

bool Stream::close() {
	PaError err;
	if (isOpen) {
		err = Pa_CloseStream(_stream);
		error_handler(err);
		isOpen = false;
		return (err == paNoError);
	}
	return true;
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
	data.left_phase = 0U;
	data.right_phase = 0U;
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
		data->wH->read_next(data->left_phase, data->right_phase);
		*out++ = data->left_phase;
		*out++ = data->right_phase;
	}
	
	return paContinue;
}

bool WAVStream::open() {
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

	if (!wH->open()) {
		return false;
	}
	if (!wH->checkHeader()) {
		return false;
	}
	if (!wH->loadDataChunk()) {
		return false;
	}
	isOpen = true;
	return true;
}
