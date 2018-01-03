// NB! DO NOT include this directly! This file should only be included from
// keyframe.h!

#include "animation/base_easing.h"

template <class T>
typename Animation<T>::elem_type Animation<T>::get(float t)
{
  if (keyframes_.empty())
    throw AnimationEmpty("::get");

  typename Keyframes::const_iterator i = keyframes_.lower_bound(t);
  if (i == keyframes_.end())
    return getTarget();
  if (i == keyframes_.begin())
    return getInitial();

  // need to interpolate between j and i
  typename Keyframes::const_iterator j = i;
  --j;

  const float t0 = j -> first;
  const float t1 = i -> first;

  // because of guarantees of map::lower_bound, t1 >= t, and t0 < t
  const float len = t1 - t0;
  const float f = (len > 0) ? ((t - t0) / len) : 1;
  const float x = (i -> second.easing) ? (i -> second.easing -> get(f)) : 0;

  return (1 - x)*j -> second.value + x*i -> second.value;
}

template <class T>
typename Animation<T>::param_type Animation<T>::getInitial() const
{
  if (keyframes_.empty())
    throw AnimationEmpty("::getInitial");

  return keyframes_.begin() -> second.value;
}

template <class T>
typename Animation<T>::param_type Animation<T>::getTarget() const
{
  if (keyframes_.empty())
    throw AnimationEmpty("::getTarget");

  return keyframes_.rbegin() -> second.value;
}

template <class T>
float Animation<T>::getElapsed(float t) const
{
  if (keyframes_.empty())
    return 0;

  return t - keyframes_.begin() -> first;
}

template <class T>
float Animation<T>::getRemaining(float t) const
{
  if (keyframes_.empty())
    return 0;

  return keyframes_.rbegin() -> first - t;
}
