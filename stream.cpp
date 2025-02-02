#include <iostream>
#include <vector>
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
	_isOpen = true;
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
	if (_isOpen) {
		err = Pa_CloseStream(_stream);
		error_handler(err);
		_isOpen = false;
		return (err == paNoError);
	}
	return true;
}

Stream::Stream() {
	_stream = nullptr;
	_isOpen = false;
}

Stream::~Stream() {
	stop();
	close();
}

/* WAVStream */

WAVStream::WAVStream(WAVHandler* wavHandler, PNFilter* filter_left, PNFilter* filter_right) {
	_wH = wavHandler;
	_data.left_phase = 0U;
	_data.right_phase = 0U;
	_data.wH = wavHandler;
	_data.filter_left = filter_left;
	_data.filter_right = filter_right;
	_stream = nullptr;
	_isOpen = false;
}

int WAVStream::callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	paWavUserData_t* _data = (paWavUserData_t*)userData;
	int16_t* out = (int16_t*)outputBuffer;
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	for (unsigned long frameIndex = 0U; frameIndex < framesPerBuffer; ++frameIndex) {
		_data->wH->read_next(_data->left_phase, _data->right_phase);
		PNFilter& filter_left = *(_data->filter_left);
		PNFilter& filter_right = *(_data->filter_right);
		*out++ = static_cast<int16_t>(filter_left(static_cast<float>(_data->left_phase)));
		*out++ = static_cast<int16_t>(filter_right(static_cast<float>(_data->right_phase)));
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
		&_data);

	error_handler(err);

	if (!_wH->open()) {
		return false;
	}
	if (!_wH->checkHeader()) {
		return false;
	}
	if (!_wH->loadDataChunk()) {
		return false;
	}
	_isOpen = true;
	return true;
}
