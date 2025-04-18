#pragma once

#include "portaudio.h"
#include "wav.hpp"
#include "filter.hpp"

/**
 * @brief Strucutre that holds left & right instant audio data that will be used as user data in the callback.
 * A pointer to a WAVHandler object is also used in callbacks to read the audio from the opened WAV file.
 */
typedef struct {
	int16_t left_phase;
	int16_t right_phase;
	WAVHandler* wH;
	PNFilter* filter_left;
	PNFilter* filter_right;
} paWavUserData_t;

/**
 * @brief Error handler for PortAudio. In case of failure, terminate PA and return false.
 */
bool errorHandlerPA(const PaError err);

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
	bool isStarted;

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
	paWavUserData_t _data;
	WAVHandler* _wH;

	static int callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
public:
	bool open();
	bool close();
	WAVStream(WAVHandler* const wavHandler, PNFilter* const filter_left, PNFilter* const filter_right);

	WAVStream() = default;
};

/**
 * @brief AudioHandle class that contains the audio objects: WAV handler, Stream and filters.
 */
class AudioHandle {
public:
	WAVHandler wH;
	PNFilter filterLeft;
	PNFilter filterRight;
	WAVStream stream;

	AudioHandle();
	~AudioHandle() = default;
};