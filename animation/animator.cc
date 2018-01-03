#include "animation/animator.h"

void Animator::update()
{
  // update the values
  float t = time_.getElapsed();
  for (Animations::iterator i = animations_.begin();
        i != animations_.end();
        ++i)
  {
    i -> second -> update(t);
  }

  // delete the animations that have ended
  Animations::iterator j = animations_.begin();
  while (j != animations_.end()) {
    if (!j -> second -> isRunning(t)) {
      Animations::iterator to_erase = j;
      ++j;
      animations_.erase(to_erase);
    } else {
      ++j;
    }
  }
}
