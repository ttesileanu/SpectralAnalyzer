#include "processor/fft.h"

#include <algorithm>
#include <vector>

#include "processor/grabber.h"
#include "processor/window_functions.h"

int FftProcessor::execute()
{
  const std::vector<float>& pdata = *boost::any_cast<const std::vector<float>*>
    (inputs_["input"] -> getOutput());
  unsigned sz = pdata.size();

  // make sure the size is right
  if (fft_.getSize() != sz)
    fft_.setSize(sz);

  if (!fft_.isInited())
    fft_.init();

  // feed the data to the FFT wrapper
  // XXX it's a little inefficient that we do this copying...
  std::copy(pdata.begin(), pdata.end(), fft_.getBuffer());
  // run the FFT
  fft_.exec();

  // fill the output structure
  output_.fft = fft_.getOutput();
  output_.size = fft_.getSize();

  // mark our cache as valid
  markValid();

  return 0;
}
