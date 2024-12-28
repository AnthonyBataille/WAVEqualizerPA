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

int Stream::callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	(void)inputBuffer;
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;
	return paContinue;
}

PaError Stream::open() {
	PaError err = Pa_OpenDefaultStream(
		&_stream,
		0,
		2,
		paFloat32,
		48000,
		paFramesPerBufferUnspecified,
		&Stream::callback,
		&data);

	error_handler(err);
	isOpen = true;
	return err;
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
	data.left_phase = 0.0f;
	data.right_phase = 0.0f;
	_stream = nullptr;
	isOpen = false;
	open();
}

Stream::~Stream() {
	stop();
	close();
}

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

	for (unsigned int i = 0; i < framesPerBuffer; ++i) {
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
	open();
}
