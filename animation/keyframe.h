/** @file keyframe.h
 *  @brief Defines various classes used for implementing keyframe animations.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef KEYFRAME_H_
#define KEYFRAME_H_

#include <map>

#include <boost/call_traits.hpp>

#include "utils/exception.h"
#include "utils/forward_defs.h"

/// Base class for all @a Animation exceptions.
class AnimationError : public Exception {
 public:
  /// Constructor.
  AnimationError(const std::string& arg) : Exception(arg) {}
};

/** @brief Exception thrown if keyframe information is queried for an
 *  @a Animation object with no keyframes.
 */
class AnimationEmpty : public AnimationError {
 public:
  /// Constructor.
  AnimationEmpty(const std::string& arg = std::string()) : AnimationError(
    "No keyframes in animation" + (arg.empty()?arg:(" (" + arg + ")")) + ".") {}
};

/// One keyframe in the animation.
template <class T>
class Keyframe {
 public:
  /// Type of the variable being animated.
  typedef T elem_type;
  /// Type to be used for parameters.
  typedef typename boost::call_traits<T>::param_type param_type;

  /// The initial value of the value controlled by the animation.
  elem_type       value;
  /// The easing function used to reach this value.
  BaseEasingPtr   easing;

  /// Empty constructor.
  Keyframe() {}
  /// Constructor with initialization, with no easing function.
  explicit Keyframe(param_type vv) : value(vv) {}
  /// Constructor with initialization.
  Keyframe(param_type vv, const BaseEasingPtr& e) : value(vv), easing(e) {}
};

/** @brief A keyframed animation for a variable of type @a T.
 *
 *  The type @a T should support addition and multiplication by a scalar of
 *  type @a float.
 */
template <class T>
class Animation {
 public:
  /// Type of the variable being animated.
  typedef T elem_type;
  /// Type to be used for parameters.
  typedef typename boost::call_traits<T>::param_type param_type;

  /// A list of keyframes.
  typedef std::map<float, Keyframe<elem_type> > Keyframes;

  /// Is the animation still running?
  bool isRunning(float t) const
    { return keyframes_.lower_bound(t) != keyframes_.end(); }

  /// Get the value of the variable at time @a t.
  elem_type get(float t);

  /// Add a keyframe at time @a t.
  void addKeyframe(float t, const Keyframe<T>& frame)
    { keyframes_[t] = frame; }

  /// Get the initial value of the animation.
  param_type getInitial() const;

  /// Get the final target value of the animation.
  param_type getTarget() const;

  /// Get the time elapsed up to @a t.
  float getElapsed(float t) const;

  /// Get the time remaining after @a t.
  float getRemaining(float t) const;
 
 private:
  Keyframes       keyframes_;
};

// the implementation has to be also in a header file
#include "keyframe-inl.h"

#endif
