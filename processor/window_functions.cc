#include "processor/window_functions.h"

#include <cmath>

#include "input/base_input.h"

int GenericWindow::execute()
{
  const std::vector<float>& data = *boost::any_cast<const std::vector<float>*>
    (inputs_["input"] -> getOutput());
  unsigned sz = data.size();

  if (windowed_.size() != sz) {
    windowed_.resize(sz);
    precalculateWindow();
  }

  for (unsigned i = 0; i < sz; ++i) {
    windowed_[i] = data[i]*window_[i];
  }

  markValid();
  return 0;
}

template <class T>
inline T sqr(T x)
{
  return x*x;
}

void GaussianWindow::precalculateWindow()
{
  unsigned sz = getSize();
  if (window_.size() != sz)
    window_.resize(sz);

  float sz2 = sz/2.0;
  for (unsigned i = 0; i < sz; ++i) {
    float x = ((float)i - sz2) / sz2;
    window_[i] = std::exp(-0.5*sqr(x/sigma_));
  }
}
