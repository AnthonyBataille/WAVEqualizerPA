#pragma once

#include "portaudio.h"

/**
 * @brief Strucutre that holds left & right instant audio data that will be used as user data in the callback.
 * The default output device is opened and used.
 */
typedef struct {
	float left_phase;
	float right_phase;
} paTestData_t;

/**
 * @brief Error handler for PortAudio. In case of failure, terminate PA and exit.
 */
void error_handler(const PaError err);

/**
 * @brief A base class of stream object that holds the underlying PaStream, open, close routines, etc...
 * The callback of the base class has no effect and must implemented in derived objects.
 */
class Stream {
protected:
	paTestData_t data;
	PaStream* _stream;
	bool isOpen;

	static int callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
	PaError open();

public:
	PaError start();
	PaError stop();
	PaError close();

	Stream();
	~Stream();
};

/**
 * @brief Saw Tooth Stream class derived from Stream class.
 * It implements a callback function taken from PA docuentation that generates the saw tooth signal.
 */
class SawToothStream: public Stream {
protected:
	static int callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
	PaError open();
public:
	SawToothStream();
};
