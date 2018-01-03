/** @file pa_input.h
 *  @brief An input module using PortAudio.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef PA_INPUT_H_
#define PA_INPUT_H_

#include <vector>

#include <portaudio.h>

#include "input/base_input.h"
#include "utils/exception.h"

/// Base class for all PortAudio exception.
class PaException : public Exception {
 public:
  /// Constructor.
  PaException(const std::string& arg) : Exception(arg) {}
};

/// Exception thrown if PortAudio fails to initialize.
class PaInitError : public PaException {
 public:
  /// Constructor.
  PaInitError(const std::string& arg = std::string()) : PaException(
    "PortAudio initialization error" + (arg.empty()?arg:(" (" + arg + ")"))
    + ".") {}
};

/// Exception thrown if no PortAudio input device is found.
class PaNoDevice : public PaException {
 public:
  /// Constructor.
  PaNoDevice(const std::string& arg = std::string()) : PaException(
    "No PortAudio device found" + (arg.empty()?arg:(" (" + arg + ")"))
    + ".") {}
};

/// Exception thrown for errors related to controlling the PortAudio stream.
class PaStreamError : public PaException {
 public:
  /// Constructor.
  PaStreamError(const std::string& arg = std::string()) : PaException(
    "PortAudio stream error" + (arg.empty()?arg:(" (" + arg + ")"))
    + ".") {}
};

/// An input module for PortAudio.
class PaInput : public BaseInput {
 public:
  /** @brief Constructor.
   *
   *  @param size The size of the window copied by @a copyWindow.
   *  @param resolution The number of samples read in at every PortAudio
   *  callback.
   */
  explicit PaInput(unsigned size, unsigned resolution = 512) : BaseInput(size),
    res_(resolution), pointer_(0), stream_(0) { }

  /// Implement the function that copies the current window into @a dest.
  virtual int copyWindow(float* dest) const;
  
  /// Implement the initialization code.
  virtual bool init();

  /// Implement the cleanup code.
  virtual void done();

 private:
  void prepareData();
  static int callback(const void* buffer, void*, unsigned long frames,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* obj);

  unsigned            res_;
  std::vector<float>  data_;
  unsigned            pointer_;
  PaStream*           stream_;
};

#endif
