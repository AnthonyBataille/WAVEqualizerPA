#include <iostream>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <string>
#include <cwchar>
#include "portaudio.h"
#include "stream.hpp"

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
	errorHandlerPA(err);
	if (err == paNoError) {
		isStarted = true;
	}
	return (err == paNoError);
}

bool Stream::stop() {
	if (_stream == nullptr) {
		return true;
	}
	PaError err = Pa_IsStreamActive(_stream);
	if (err == 1) {
		err = Pa_StopStream(_stream);
		errorHandlerPA(err);
		isStarted = false;
	}
	else if (err < paNoError) {
		errorHandlerPA(err);
	}
	return (err == paNoError);
}

bool Stream::close() {
	PaError err;
	if (isOpen) {
		err = Pa_CloseStream(_stream);
		errorHandlerPA(err);
		_stream = nullptr;
		isOpen = false;
		return (err == paNoError);
	}
	return true;
}

Stream::Stream() {
	_stream = nullptr;
	isOpen = false;
	isStarted = false;
}

Stream::~Stream() {
	stop();
	close();
}

/* WAVStream */

WAVStream::WAVStream(WAVHandler* const wavHandler, PNFilter* const filter_left, PNFilter* const filter_right) {
	_wH = wavHandler;
	_data.left_phase = 0U;
	_data.right_phase = 0U;
	_data.wH = wavHandler;
	_data.filter_left = filter_left;
	_data.filter_right = filter_right;
	_stream = nullptr;
	isOpen = false;
	isStarted = false;
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

	errorHandlerPA(err);

	if (!_wH->open()) {
		return false;
	}
	if (!_wH->checkHeader()) {
		return false;
	}
	if (!_wH->loadDataChunk()) {
		return false;
	}
	isOpen = true;
	return true;
}

bool WAVStream::close() {
	_wH->close();
	return Stream::close();
}

/* AudioHandle */

AudioHandle::AudioHandle() : wH(),  filterLeft(), filterRight(), stream(&wH, &filterLeft, &filterRight) {}
