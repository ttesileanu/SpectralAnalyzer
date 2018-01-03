/** @file base_easing.h
 *  @brief Defines the interface for easing functions. These are used in
 *  animations, to interpolate between different keyframes.
 *
 *  The design of this is partly inspired by the animation subpart of the
 *  Qt framework.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef BASE_EASING_H_
#define BASE_EASING_H_

/** @brief This class defines the interface for an easing function.
 *
 *  An easing function is just a way to interpolate between 0 and 1, as a
 *  function of "time" -- a variable that runs from 0 to 1. This is used in
 *  keyframe animation to provide a smooth transition between the keys.
 */
class BaseEasing {
 public:
  /// Need a virtual destructor, for inheritance.
  virtual ~BaseEasing() {}

  /** @brief Given a "time" between 0 and 1, return the value of the easing
   *  function at that time.
   *
   *  The output must be such that @a get(0) == 0 and $a get(1) == 1, but
   *  otherwise the intermediate values are not necessarily constrained to
   *  the interval [0, 1] (though they should stay close). This allows for
   *  various overshooting functions.
   */
  virtual float get(float x) const = 0;
};

#endif
