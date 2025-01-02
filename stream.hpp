#pragma once

#include "portaudio.h"
#include "wav.hpp"

/**
 * @brief Strucutre that holds left & right instant audio data that will be used as user data in the callback.
 * A pointer to a WAVHandler object is also used in callbacks to read the audio from the opened WAV file.
 */
typedef struct {
	int16_t left_phase;
	int16_t right_phase;
	WAVHandler* wH;
} paWavUserData_t;

/**
 * @brief Error handler for PortAudio. In case of failure, terminate PA and return false.
 */
bool error_handler(const PaError err);

/**
 * @brief A base class of stream object that holds the underlying PaStream, open, close routines, etc...
 * The callback of the base class has no effect and must implemented in derived objects.
 */
class Stream {
protected:
	PaStream* _stream;

	static int callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
public:
	bool isOpen;

	bool start();
	bool stop();
	bool open();
	bool close();

	Stream();
	~Stream();
};

/**
 * @brief Derived class from Stream that makes use of WAVHandler to read the audio from a  WAV file.
 * The WAV file has 1 channel with encoded data in signed PCM 16 bits (2 bytes per block).
 * The default output device is opened and used.
 */
class WAVStream: public Stream {
protected:
	paWavUserData_t data;
	WAVHandler* wH;

	static int callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
public:
	bool open();
	WAVStream(WAVHandler* wavHandler);

	WAVStream() = default;
};
