#include "stream_test.hpp"

#include <iostream>

#include "portaudio.h"

/* SawToothStream */

int SawToothStream::callback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData) {
  paTestData_t *_data = (paTestData_t *)userData;
  float *out = (float *)outputBuffer;
  (void)inputBuffer;
  (void)timeInfo;
  (void)statusFlags;

  for (unsigned long i = 0; i < framesPerBuffer; ++i) {
    *out++ = _data->leftPhase;
    *out++ = _data->rightPhase;
    _data->leftPhase += 0.001f;
    if (_data->leftPhase >= 1.0f) {
      _data->leftPhase = -1.0f;
    }

    _data->rightPhase += 0.001f;
    if (_data->rightPhase >= 2.0f) {
      _data->rightPhase = -1.0f;
    }
  }

  return paContinue;
}

bool SawToothStream::open() {
  PaError err = Pa_OpenDefaultStream(&_stream, 0, 2, paFloat32, 48000,
                                     paFramesPerBufferUnspecified,
                                     &SawToothStream::callback, &_data);

  errorHandlerPA(err);
  if (err == paNoError) {
    _isOpen = true;
    return true;
  }
  return false;
}

SawToothStream::SawToothStream() {
  _data.leftPhase = 0.0f;
  _data.rightPhase = 0.0f;
  _stream = nullptr;
  _isOpen = false;
}

void runTestStream() {
  SawToothStream defaultStream;

  if (!defaultStream.open()) {
    std::cerr << "Unable to open the stream." << std::endl;
    return;
  }

  std::cout << "Press Enter to start." << std::endl;
  std::cin.get();

  if (!defaultStream.start()) {
    std::cerr << "Unable to start the stream." << std::endl;
    return;
  }

  std::cout << "Press Enter to stop." << std::endl;
  std::cin.get();

  if (!defaultStream.stop()) {
    std::cerr << "Unable to stop the stream." << std::endl;
    return;
  }

  if (!defaultStream.close()) {
    std::cerr << "Unable to close the stream." << std::endl;
  }
}
