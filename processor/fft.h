/** @file fft.h
 *  @brief Defines an FFT processing module.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef FFT_H_
#define FFT_H_

#include <vector>

#include "processor/base_processor.h"
#include "processor/fftwrapper.h"

/// The complex number type used by the FFT routines.
typedef RealFft::Complex Complex;

/** @brief This defines a module that performs FFT on the input data.
 *
 *  The module should have one input, called "input", which should provide it
 *  with the data on which to perform the FFT. The default output of the module
 *  (obtainable by getBuffer) is empty; instead, the output should be gotten
 *  by using getFft.
 */
class FftProcessor : public BaseProcessor {
 public:
  struct OutputStruct {
    const Complex*    fft;
    unsigned          size;
  };
  typedef const OutputStruct* Output;

 protected:
  /// Calculate the FFT.
  virtual int execute();

  /// Return the FFT data.
  boost::any getOutput_() const { return &output_; }

 private:
  RealFft           fft_;
  OutputStruct      output_;
};

#endif
