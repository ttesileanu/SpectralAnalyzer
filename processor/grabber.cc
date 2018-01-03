#include "processor/grabber.h"

int Grabber::execute()
{
  if (!backend_)
    return 1;

  const unsigned sz = backend_ -> getWindowSize();
  // make sure our data vector has the right size
  if (data_.size() != sz)
    data_.resize(sz);

  details_.samplingFrequency = backend_ -> getSamplingFrequency();
  details_.size = sz;

  // get the data
  int res = backend_ -> copyWindow(&data_[0]);
  if (res != 0)
    return res;
  else {
    markValid();
    return 0;
  }
}
