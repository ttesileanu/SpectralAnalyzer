/** @file fake_input.h
 *  @brief Defines an input module that generates fake data.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef FAKE_INPUT_H_
#define FAKE_INPUT_H_

#include "input/base_input.h"
#include "utils/misc.h"

/// An input module that generates a sinusoidal wave.
class FakeInput : public BaseInput {
 public:
  /// Constructor.
  explicit FakeInput(unsigned size) : BaseInput(size), freq_(440),
    amp_(1), phi_(0) {}

  /// Implement the function that copies the current window into @a dest.
  virtual int copyWindow(float* dest) const;

  /// Set the wave's frequency.
  void setFrequency(float f) { freq_ = f; }

  /// Set the wave's amplitude.
  void setAmplitude(float a) { amp_ = a; }

  /** @brief Initialize the sound input.
   *
   *  Return @a true for success.
   */
  virtual bool init() {
    if (!BaseInput::init())
      return false;

    freq_ = properties_ -> get("frequency", 0.0);
    return true;
  }

 private:
  Timer           timer_;
  float           freq_;
  float           amp_;
  mutable float   phi_;
};

#endif
