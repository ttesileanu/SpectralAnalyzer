/** @file window_functions.h
 *  @brief Defines various processors that implement various window functions,
 *  for use with FFT.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef WINDOW_FUNCTIONS_H_
#define WINDOW_FUNCTIONS_H_

#include <vector>

#include "processor/base_processor.h"
#include "processor/grabber.h"

/** @brief Defines a generic window function.
 *
 *  This takes its input from the input module called "input".
 */
class GenericWindow : public BaseProcessor {
 public:
  typedef Grabber::Details Details;
  typedef const std::vector<float>* Output;

 protected:
  /// Calculate the windowed result.
  virtual int execute();

  /// Get access to the windowed result.
  boost::any getOutput_() const { return &windowed_; }

  boost::any getDetails_() const {
    Inputs::const_iterator i = inputs_.find("input");
    return i -> second -> getDetails();
  }

  /// This should be implemented by descendants.
  virtual void precalculateWindow() = 0;

  // a precalculated window function
  std::vector<float>      window_;

  size_t getSize() const { return windowed_.size(); }

 private:
  // the windowed output
  std::vector<float>      windowed_;
};

class GaussianWindow : public GenericWindow {
 public:
  /// Constructor.
  explicit GaussianWindow(float s = 0.5) : sigma_(s) {}

  /// Set the standard deviation for the gaussian.
  void setStd(float s) { sigma_ = s; }

 protected:
  virtual void precalculateWindow();

 private:
  float         sigma_;
};

#endif
