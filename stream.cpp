#include <array>
#include <iostream>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cwchar>
#include <iostream>
#include <limits>
#include <string>

#include "portaudio.h"
#include "stream.hpp"

constexpr static float LIMITER_THRESHOLD =
    static_cast<float>(std::numeric_limits<int16_t>::max()) * 0.9f;

/* Stream */

int Stream::callback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags, void *userData) {
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
  } else if (err < paNoError) {
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

WAVStream::WAVStream(WAVHandler *const wavHandler,
                     std::array<PNFilter, NUM_FILTERS> &filtersLeft,
                     std::array<PNFilter, NUM_FILTERS> &filterRight) {
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

int WAVStream::callback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userData) {
  paWavUserData_t *_data = (paWavUserData_t *)userData;
  int16_t *out_left = (int16_t *)outputBuffer;
  int16_t *out_right = out_left + 1;
  (void)inputBuffer;
  (void)timeInfo;
  (void)statusFlags;
  for (unsigned long frameIndex = 0U; frameIndex < framesPerBuffer;
       ++frameIndex) {
    _data->wH->read_next(_data->leftPhase, _data->rightPhase);
    *out_left = _data->leftPhase;
    *out_right = _data->rightPhase;
    for (size_t i = 0; i < NUM_FILTERS; ++i) {
      PNFilter &filtersLeft = _data->filtersLeft->at(i);
      PNFilter &filterRight = _data->filterRight->at(i);
      float left = filtersLeft(static_cast<float>(*out_left));
      float right = filterRight(static_cast<float>(*out_right));

      left = limiter(left, LIMITER_THRESHOLD);
      right = limiter(right, LIMITER_THRESHOLD);
      *out_left = static_cast<int16_t>(left);
      *out_right = static_cast<int16_t>(right);
    }
    out_left += 2;
    out_right += 2;
  }
  return paContinue;
}

bool WAVStream::open() {
  PaError err = Pa_OpenDefaultStream(&_stream, 0, 2, paInt16, 48000,
                                     paFramesPerBufferUnspecified,
                                     &WAVStream::callback, &_data);

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

AudioHandle::AudioHandle()
    : Gains(), wH(), filtersLeft(), filtersRight(),
      stream(&wH, filtersLeft, filtersRight) {
  for (float &g : Gains) {
    g = 1.0f;
  }
}
