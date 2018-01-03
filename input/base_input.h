/** @file base_input.h
 *  @brief Define an interface for input modules.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef BASE_INPUT_H_
#define BASE_INPUT_H_

#include "utils/properties.h"

/** @brief Interface for input modules.
 *
 *  Input modules model a sliding time window whose contents can be accessed
 *  at any time, asynchronously with the sliding.
 */
class BaseInput {
 public:
  /// Virtual destructor, for proper inheritance.
  virtual ~BaseInput() {}

  /** @brief Copies the current window into @a dest.
   *
   *  Should return 0 for success. To be implemented by descendants.
   */
  virtual int copyWindow(float* dest) const = 0;

  /// Get window size.
  unsigned getWindowSize() const { return win_size_; }

  /** @brief Initialize the sound input.
   *
   *  Return @a true for success.
   */
  virtual bool init() {
    if (!properties_)
      return false;

    samp_freq_ = properties_ -> get<unsigned>("rate");
    return true;
  }

  /// Clean up.
  virtual void done() {}

  /// Get sampling frequency.
  unsigned getSamplingFrequency() const { return samp_freq_; }

  /** @brief Set the sampling frequency.
   *
   *  This may reset the input. Descendants should override this to add
   *  additional processing, if needed.
   */
  virtual void setSamplingFrequency(unsigned f) { samp_freq_ = f; }

  /// Give the module access to its settings.
  void setProperties(Properties* props) { properties_ = props; }

  /// Update the settings. Descendants should implement this. @see setProperties
  virtual void updateProperties() {}

 protected:
  // we can't instantiate this anyway
  explicit BaseInput(unsigned sz) : properties_(0), win_size_(sz),
    samp_freq_(44100) {}

  Properties*   properties_;

 private:
  unsigned      win_size_;
  unsigned      samp_freq_;
};

#endif
