#include <iostream>
#include <vector>
#include <array>

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

WAVStream::WAVStream(WAVHandler* const wavHandler, std::array<PNFilter, NUM_FILTERS>& filtersLeft, std::array<PNFilter, NUM_FILTERS>& filterRight) {
	_wH = wavHandler;
	_data.leftPhase = 0U;
	_data.rightPhase = 0U;
	_data.wH = wavHandler;
	_data.filtersLeft = &filtersLeft;
	_data.filterRight = &filterRight;
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
	int16_t* out_left = (int16_t*)outputBuffer;
	int16_t* out_right = out_left + 1;
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	for (unsigned long frameIndex = 0U; frameIndex < framesPerBuffer; ++frameIndex) {
		_data->wH->read_next(_data->leftPhase, _data->rightPhase);
		*out_left = _data->leftPhase;
		*out_right = _data->rightPhase;
		for (size_t i = 0; i < NUM_FILTERS; ++i) {
			PNFilter& filtersLeft = _data->filtersLeft->at(i);
			PNFilter& filterRight = _data->filterRight->at(i);
			*out_left = static_cast<int16_t>(filtersLeft(static_cast<float>(*out_left)));
			*out_right = static_cast<int16_t>(filterRight(static_cast<float>(*out_right)));
		}
		out_left += 2;
		out_right += 2;
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

AudioHandle::AudioHandle() : Gains(), wH(), filtersLeft(), filtersRight(), stream(&wH, filtersLeft, filtersRight) {
	for (float& g : Gains){
		g = 1.0f;
	}
}
