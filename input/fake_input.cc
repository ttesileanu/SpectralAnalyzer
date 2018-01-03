#include "input/fake_input.h"

#include <cmath>

int FakeInput::copyWindow(float* dest) const
{
  const float omega = 2*M_PI*freq_;

  unsigned sz = getWindowSize();
  const float dt = 1.0/getSamplingFrequency();
  const float t = timer_.getElapsed();
  phi_ += omega*t;
  phi_ = phi_ - 2*M_PI*std::floor(phi_/(2*M_PI));
  for (unsigned i = 0; i < sz; ++i) {
    const float x = i*dt;
    dest[i] = amp_*std::sin(phi_ + omega*x);
  }

  return 0;
}
