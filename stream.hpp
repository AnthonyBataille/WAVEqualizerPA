#pragma once

#include <array>

#include "filter.hpp"
#include "portaudio.h"
#include "wav.hpp"

constexpr int NUM_FILTERS = 10;

constexpr std::array<float, NUM_FILTERS> CENTER_FREQUENCIES{
    31.5f,   63.0f,   125.0f,  250.0f,  500.0f,
    1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f};

constexpr std::array<float, NUM_FILTERS> BANDWIDTHS{
    22.0f,  44.0f,   89.0f,   178.0f,  355.0f,
    710.0f, 1420.0f, 2840.0f, 5680.0f, 11360.0f};

/**
 * @brief Strucutre that holds left & right instant audio data that will be used
 * as user data in the callback. A pointer to a WAVHandler object is also used
 * in callbacks to read the audio from the opened WAV file.
 */
typedef struct {
  int16_t leftPhase;
  int16_t rightPhase;
  WAVHandler *wH;
  std::array<PNFilter, NUM_FILTERS> *filtersLeft;
  std::array<PNFilter, NUM_FILTERS> *filterRight;
} paWavUserData_t;

/**
 * @brief Error handler for PortAudio. In case of failure, terminate PA and
 * return false.
 */
bool errorHandlerPA(const PaError err);

/**
 * @brief A base class of stream object that holds the underlying PaStream,
 * open, close routines, etc... The callback of the base class has no effect and
 * must implemented in derived objects.
 */
class Stream {
protected:
  PaStream *_stream;

  static int callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData);

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
 * @brief Derived class from Stream that makes use of WAVHandler to read the
 * audio from a  WAV file. The WAV file has 1 channel with encoded data in
 * signed PCM 16 bits (2 bytes per block). The default output device is opened
 * and used.
 */
class WAVStream : public Stream {
protected:
  paWavUserData_t _data;
  WAVHandler *_wH;

  static int callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData);

public:
  bool open();
  bool close();
  WAVStream(WAVHandler *const wavHandler,
            std::array<PNFilter, NUM_FILTERS> &filtersLeft,
            std::array<PNFilter, NUM_FILTERS> &filterRight);

  WAVStream() = default;
};

/**
 * @brief AudioHandle class that contains the audio objects: WAV handler, Stream
 * and filters.
 */
class AudioHandle {
public:
  std::array<float, NUM_FILTERS> Gains;
  WAVHandler wH;
  std::array<PNFilter, NUM_FILTERS> filtersLeft;
  std::array<PNFilter, NUM_FILTERS> filtersRight;
  WAVStream stream;

  AudioHandle();
  ~AudioHandle() = default;
};
