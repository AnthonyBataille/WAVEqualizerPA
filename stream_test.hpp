#pragma once

#include "portaudio.h"
#include "stream.hpp"
#include "wav.hpp"

/**
 * @brief Strucutre that holds left & right instant audio data that will be used
 * as user data in the callback. The default output device is opened and used.
 */
typedef struct {
  float leftPhase;
  float rightPhase;
} paTestData_t;

/**
 * @brief Saw Tooth Stream class derived from Stream class.
 * It implements a callback function taken from PA docuentation that generates
 * the saw tooth signal.
 */
class SawToothStream : public Stream {
 protected:
  paTestData_t _data;
  static int callback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags, void* userData);

 public:
  bool open();
  SawToothStream();
};
