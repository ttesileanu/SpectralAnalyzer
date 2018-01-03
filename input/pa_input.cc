#include "pa_input.h"

#include <algorithm>

#include <cmath>

int PaInput::copyWindow(float* dest) const
{
  const unsigned size = getWindowSize();
  // just in case the portaudio callback runs in a different thread...
  unsigned pointer = pointer_;

  unsigned s_dest;
  unsigned s_src;
  if (pointer < size) {
    unsigned diff = size - pointer;
    std::copy(data_.end() - diff, data_.end(), dest);

    s_dest = diff;
    s_src = 0;
  } else {
    s_src = pointer - size;
    s_dest = 0;
  }

  std::copy(data_.begin() + s_src, data_.begin() + pointer, dest + s_dest);

  return 0;
}

bool PaInput::init()
{
  // resize the buffer, and fill it with zeros
  prepareData();

  // initialize PA
  PaError err = paNoError;
  err = Pa_Initialize(); // XXX what happens if we call this twice?
  if (err != paNoError)
    throw PaInitError();

  // get a device
  PaStreamParameters params;

  params.device = Pa_GetDefaultInputDevice();
  if (params.device == paNoDevice)
    throw PaNoDevice();

  params.channelCount = 1;
  params.sampleFormat = paFloat32;
  params.suggestedLatency = Pa_GetDeviceInfo(params.device)
    -> defaultLowInputLatency;
  params.hostApiSpecificStreamInfo = 0;

  // start recording
  err = Pa_OpenStream(&stream_, &params, 0, getSamplingFrequency(), res_,
    paClipOff, &PaInput::callback, this);
  if (err != paNoError)
    throw PaStreamError("open failed");

  err = Pa_StartStream(stream_);
  if (err != paNoError)
    throw PaStreamError("start failed");

  return true;
}

void PaInput::done()
{
  PaError err = Pa_CloseStream(stream_);
  if (err != paNoError)
    throw PaStreamError("close failed");

  Pa_Terminate();
}

void PaInput::prepareData()
{
  unsigned buf_size = getWindowSize();
  buf_size = res_*std::ceil((float)buf_size / res_);

  data_.clear();
  data_.resize(buf_size, 0);

  pointer_ = 0;
}

int PaInput::callback(const void* buffer_v, void*, unsigned long frames,
  const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* obj_v)
{
  PaInput* obj = (PaInput*)obj_v;
  const float* buffer = (const float*)buffer_v;

  if (buffer) {
    std::copy(buffer, buffer + frames, obj -> data_.begin() + obj -> pointer_);
  } else {
    std::fill(obj -> data_.begin() + obj -> pointer_,
      obj -> data_.begin() + obj -> pointer_ + frames, 0);
  }

  obj -> pointer_ += frames;
  const size_t sz = obj -> data_.size();
  if (obj -> pointer_ >= sz)
    obj -> pointer_ -= sz;

  return paContinue;
}
