#include "standard_easing.h"

float StandardEasing::get(float x) const
{
  // for some cases, the subtype doesn't matter
  switch (type_) {
    case NONE: return 0;
    case LINEAR: return getLinear(x);
    default:;
  }

  // now handle subtypes
  switch (subtype_) {
    case IN:
      return simpleGet(x);
    case OUT:
      return 1 - simpleGet(1 - x);
    case INOUT:
      if (x <= 0.5)
        return 0.5*simpleGet(2*x);
      else
        return 0.5*(2 - simpleGet(2 - 2*x));
    case OUTIN:
      if (x <= 0.5)
        return 0.5*(1 - simpleGet(1 - 2*x));
      else
        return 0.5*(1 + simpleGet(2*x - 1));
    default: // this should never happen!
      return 0;
  }
}

float StandardEasing::simpleGet(float x) const
{
  switch (type_) {
    case NONE:
      return 0;
    case LINEAR:
      return getLinear(x);
    case QUADRATIC:
      return getQuadratic(x);
    case POWER:
      return getPower(x);
    case SINE:
      return getSine(x);
    default: // this should never happen!
      return 0;
  }
}

float CompositeEasing::get(float t) const
{
  if (functions_.empty())
    return 1;

  Functions::const_iterator i = functions_.lower_bound(t);
  if (i == functions_.end())
    return 1;
  
  float t1 = i -> first;
  float x1 = i -> second.first;
  float t0, x0;
  if (i == functions_.begin()) {
    t0 = 0;
    x0 = 0;
  } else {
    Functions::const_iterator j = i;
    --j;
    t0 = j -> first;
    x0 = j -> second.first;
  }

  float normalized_t = (t - t0) / (t1 - t0);
  float val = i -> second.second -> get(normalized_t);
  
  return x0 + (x1 - x0)*val;
}
