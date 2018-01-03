/** @file standard_easing.h
 *  @brief Defines a class that implements several standard easing functions.
 *
 *  These are inspired by the easing functions in the Qt animation framework.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef STANDARD_EASING_H_
#define STANDARD_EASING_H_

#include <map>
#include <utility>

#include <cmath>

#include "animation/base_easing.h"
#include "utils/forward_defs.h"

/** @brief This class can implement a variety of easing functions.
 *
 *  Here are the options (the possible values of the @a Type @a enum):
 *    - @a NONE: abrupt transition
 *    - @a LINEAR: linear interpolation
 *    - @a QUADRATIC: quadratic interpolation
 *    - @a POWER: interpolation with a power function of arbitrary exponent
 *    - @a SINE: interpolation with a sine function
 *
 *  For each of these, we can choose an IN, OUT, INOUT, or OUTIN animation.
 *  The meaning of these terms can depend on the type of the animation, but
 *  generically:
 *    - @a IN has a positive acceleration throughout
 *    - @a OUT has a negative acceleration
 *    - @a INOUT starts with a positive acceleration, then decelerates
 *    - @a OUTIN starts with a negative acceleration, then accelerates
 *
 *  Note that for @a LINEAR there is no distinction between these subtypes.
 */
class StandardEasing : public BaseEasing {
 public:
  /// The type of the easing function.
  enum Type { NONE = 0, LINEAR, QUADRATIC, POWER, SINE };
  enum Subtype { IN, OUT, INOUT, OUTIN };

  /// Empty constructor -- abrupt easing function.
  StandardEasing() : type_(NONE), subtype_(IN), power_(0), subpower_(1),
    straight_(0), freq_(M_PI/2) {}
  /** @brief Constructor with initialization of transition type.
   *
   *  The second parameter, @a power, is ignored unless @a t == @a POWER.
   *  If @a t == @a POWER, then @a power is the exponent of the power function
   *  used for the transition. Using type @a t == @a POWER and @a power == 1
   *  is equivalent to @a t == @a LINEAR; similarly, if @a power == 2, we get
   *  an equivalent to @a t == @a QUADRATIC, and if @a power == 0, we get
   *  an equivalent to @a t == @a NONE. Note, however, that the more specific
   *  types (@a NONE, @a LINEAR, @a QUADRATIC) might be more efficient than
   *  the generic @a POWER.
   */
  StandardEasing(Type t, float power = 0) : type_(t), subtype_(IN),
    power_(power), subpower_(1), straight_(0), freq_(M_PI/2) {}

  /// Constructor with initialization of transition type and subtype.
  StandardEasing(Type t, Subtype st) : type_(t), subtype_(st),
    power_(0), subpower_(1), straight_(0), freq_(M_PI/2) {}

  /** @brief Change the subtype (in/out/in-out/out-in transition).
   *
   *  This returns @a this to facilitate stringing of the setter functions
   *  in an expression.
   */
  StandardEasing* setSubtype(Subtype s) { subtype_ = s; return this; }

  /** @brief Change the power (for a @a POWER transition).
   *
   *  This returns @a this to facilitate stringing of the setter functions
   *  in an expression.
   */
  StandardEasing* setPower(float p) { power_ = p; return this; }

  /** @brief Change the subpower (for a @a POWER transition).
   *
   *  A power transition has two terms: one with exponent given by the power,
   *  and one with exponent given by the subpower. This function sets the
   *  subpower exponent.
   *
   *  This returns @a this to facilitate stringing of the setter functions
   *  in an expression.
   */
  StandardEasing* setSubpower(float p) { subpower_ = p; return this; }

  /** @brief Change the "straightness" of the transition (for @a QUADRATIC
   *  and @a POWER).
   *
   *  This sets the amount of subpower to use for power transition. For
   *  @a QUADRATIC, the subpower exponent is always 1.
   *
   *  This returns @a this to facilitate stringing of the setter functions
   *  in an expression.
   */
  StandardEasing* setStraightness(float s) { straight_ = s; return this; }

  /** @brief Change the frequency for sine-related transitions.
   *
   *  This returns @a this to facilitate stringing of the setter functions
   *  in an expression.
   */
  StandardEasing* setFrequency(float w) { freq_ = w; return this; }

  /// Implement the actual calculation.
  virtual float get(float x) const;

 private:
  float simpleGet(float x) const;

  float getLinear(float x) const { return x; }
  float getQuadratic(float x) const { return x*(x+straight_)/(1+straight_); }
  float getPower(float x) const
    { return std::pow(x, subpower_)*(std::pow(x, power_-subpower_)+straight_)
              / (1 + straight_); }
  float getSine(float x) const { return std::sin(freq_*x) / std::sin(freq_); }

  Type          type_;
  Subtype       subtype_;
  float         power_;
  float         subpower_;
  float         straight_;
  float         freq_;
};

/// This class can string several easing functions together.
class CompositeEasing : public BaseEasing {
 public:
  /** @brief Add an easing function.
   *
   *  The 'time' @a t must be between 0 and 1. The easing function @a f
   *  will be used from the previous time point to @a t. Here, 'previous time
   *  point' means the largest time smaller than @a t that was added to the
   *  composite easing function, or zero, if none exists.
   *
   *  The value @a x is the value to be reached by the easing function by
   *  time @a t. As with all easing functions, the values should be kept
   *  close to the interval [0, 1], but may stray out of this interval for
   *  special effects. The value of @a x at @a t = 0 is assumed to be zero.
   *
   *  If the maximum time added is smaller than 1, an abrupt change occurs
   *  at that time.
   */
  CompositeEasing* add(float t, float x, const BaseEasingPtr& f)
    { functions_[t] = std::make_pair(x, f); return this; }

  /// Implement the actual calculation.
  virtual float get(float t) const;

 private:
  /// Checkpoint -- value to reach, and easing function to use.
  typedef std::pair<float, BaseEasingPtr> Checkpoint;
  /// Times up to which easing functions end.
  typedef std::map<float, Checkpoint> Functions;

  /// The easing functions to be used.
  Functions functions_;
};

#endif
