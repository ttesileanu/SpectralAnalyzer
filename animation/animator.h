/** @file animator.h
 *  @brief Defines a class that manages the animation of several variables.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef ANIMATOR_H_
#define ANIMATOR_H_

#include <map>

#include <boost/any.hpp>
#include <boost/call_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "animation/keyframe.h"
#include "utils/exception.h"
#include "utils/misc.h"

/// Base class for all @a Animator exceptions.
class AnimatorError : public Exception {
 public:
  /// Constructor.
  AnimatorError(const std::string& arg) : Exception(arg) {}
};

/// Exception thrown when trying to access an unknown animator variable.
class AnimatorUnknownVariable : public AnimatorError {
 public:
  /// Constructor.
  AnimatorUnknownVariable(const std::string& arg = std::string()) :
    AnimatorError("Unknown animator variable" + (arg.empty()?arg:
    (" (" + arg + ")")) + ".") {}
};

/** @brief This class manages keyframed animations.
 *
 *  It can handle any type that supports addition and multiplication by
 *  @a float. It updates these values in-place upon calling the method
 *  @a update.
 */
class Animator {
 public:
  /** @brief Add a keyframe to an animation.
   *
   *  If an animation for the given variable does not already exist, one
   *  is created. For the initial state, use @a dt = 0 and provide no
   *  (empty pointer) easing function.
   *
   *  @param variable Pointer to the variable to be animated.
   *  @param dt The length of time by which the animation should be finished,
   *  measured (in seconds) from the current time.
   *  @param value The value that the variable should reach in time @a dt.
   *  @param easing The easing transition to be used. An empty pointer means no
   *  transition.
   */
  template <class T>
  void addKeyframe(T* variable, float dt,
      typename boost::call_traits<T>::param_type value,
      BaseEasingPtr easing = BaseEasingPtr()) {
    if (dt == 0)
      *variable = value;
    float t = time_.getElapsed() + dt;

    if (animations_.count(variable) == 0) {
      animations_.insert(std::make_pair(variable,
        boost::make_shared<AnimationData<T> >(variable)));
    }
    getAnimation_<T>(animations_[variable]).
      addKeyframe(t, Keyframe<T>(value, easing));
  }

  /** @brief Replace any current animation with the one given.
   *
   *  If an animation for the variable already exists, it is deleted.
   */
  template <class T>
  void doTransition(T* variable,
      typename boost::call_traits<T>::param_type start,
      typename boost::call_traits<T>::param_type end,
      const std::pair<float, BaseEasingPtr>& trans) {
    deleteAnimation(variable);
    // XXX what if start & end were references obtained by getTarget?
    // the deletion would invalidate those!...
    if (trans.first > 0) {
      addKeyframe(variable, 0, start);
      addKeyframe(variable, trans.first, end, trans.second);
    } else {
      *variable = end;
    }
  }

  /** @brief Replace any current animation with the one given.
   *
   *  If an animation for the variable already exists, it is deleted, but the
   *  new animation always starts from the current value of the variable, and
   *  the length of the animation is shortened by the remaining time in the
   *  previous animation.
   */
  template <class T>
  void redoTransition(T* variable,
      typename boost::call_traits<T>::param_type end,
      const std::pair<float, BaseEasingPtr>& trans) {
    float len = trans.first;
    if (isAnimated(variable)) {
      len -= getRemaining(variable);
      if (len < 0)
        len = 0;
      deleteAnimation(variable);
    }
    if (len > 0) {
      addKeyframe(variable, 0, *variable);
      addKeyframe(variable, len, end, trans.second);
    } else {
      *variable = end;
    }
  }


  /** @brief Get the value of the variable.
   *
   *  If @a instantaneous is @a true, this simply directly reads @a *variable.
   *  Otherwise, it calls @a getTarget to get the final target value for the
   *  variable.
   */
  template <class T>
  typename boost::call_traits<T>::param_type
      get(const T* variable, bool instantaneous = false) const {
    return instantaneous?(*variable):getTarget(variable);
  }

  /// Check whether a certain animation exists.
  template <class T>
  bool isAnimated(const T* variable) const
    { return animations_.find(variable) != animations_.end(); }

  /// Delete an animation.
  template <class T>
  void deleteAnimation(const T* variable)
    { animations_.erase(variable); }

  /// Get the initial value of the animated variable.
  template <class T>
  typename boost::call_traits<T>::param_type
      getInitial(const T* variable) const {
    Animations::const_iterator i = animations_.find(variable);
    if (i == animations_.end())
      throw AnimatorUnknownVariable("::getInitial");

    return getAnimation_<T>(i -> second).getInitial();
  }
  
  /// Get the final target value. Read @a *variable if it isn't animated.
  template <class T>
  typename boost::call_traits<T>::param_type
      getTarget(const T* variable) const {
    Animations::const_iterator i = animations_.find(variable);
    if (i == animations_.end())
      return *variable;

    return getAnimation_<T>(i -> second).getTarget();
  }

  /// Get the elapsed time of the animation.
  template <class T>
  float getElapsed(const T* variable) const {
    Animations::const_iterator i = animations_.find(variable);
    if (i == animations_.end())
      throw AnimatorUnknownVariable("::getElapsed");

    float t = time_.getElapsed();
    return getAnimation_<T>(i -> second).getElapsed(t);
  }

  /// Get the remaining time for the animation.
  template <class T>
  float getRemaining(const T* variable) const {
    Animations::const_iterator i = animations_.find(variable);
    if (i == animations_.end())
      return 0;

    float t = time_.getElapsed();
    return getAnimation_<T>(i -> second).getRemaining(t);
  }

  /** @brief Update the animated variables.
   *
   *  This also updates the list of animations, removing the ones that
   *  have finished.
   */
  void update();

 private:
  /// Base class for structure containing animated variable information.
  class BaseAnimationData {
   public:
    /// Empty constructor.
    BaseAnimationData() {}
    /// Constructor with initialization.
    template <class T>
    explicit BaseAnimationData(T* variable) { variable_ = variable; }

    /// Virtual destructor, for proper inheritance.
    virtual ~BaseAnimationData() {}

    /// Access the variable.
    boost::any getVariable() const { return variable_; }

    /// Access the animation.
    virtual boost::any getAnimation() = 0;

    /// Access the animation (read-only).
    virtual boost::any getAnimation() const = 0;

    /// Update the animation.
    virtual void update(float t) = 0;

    /// Check whether the animation is still running.
    virtual bool isRunning(float t) = 0;
   
   private:
    /// Boost::any object containing a pointer to the animated variable.
    boost::any        variable_;
  };

  /// Structure containing the information for one animated variable.
  template <class T>
  class AnimationData : public BaseAnimationData {
   public:
    explicit AnimationData(T* varPtr) : BaseAnimationData(varPtr) {}
    AnimationData(T* varPtr, const Animation<T>& keys) :
        BaseAnimationData(varPtr), keyframes_(keys) {}

    /// Access the animation.
    virtual boost::any getAnimation() { return &keyframes_; }

    /// Access the animation (read-only).
    virtual boost::any getAnimation() const { return &keyframes_; }

    /// Get pointer to the variable.
    T* getVarPtr() { return boost::any_cast<T*>(getVariable()); }

    /// Update the animation.
    virtual void update(float t) {
      *(getVarPtr()) = keyframes_.get(t);
    }

    /// Check whether the animation is still running.
    virtual bool isRunning(float t) { return keyframes_.isRunning(t); }

   private:
    /// The keyframed animation for the variable.
    Animation<T>        keyframes_;
  };

  /// A smart pointer to a @a BaseAnimationData object.
  typedef boost::shared_ptr<BaseAnimationData> BaseAnimationDataPtr;

  /// Access a specific animation.
  template <class T>
  Animation<T>& getAnimation_(const BaseAnimationDataPtr& anim) {
    return *(boost::any_cast<Animation<T>*>(anim -> getAnimation()));
  }

  /// Access a specific animation read-only-ly.
  template <class T>
  const Animation<T>& getAnimation_(const BaseAnimationDataPtr& anim) const {
    return *(boost::any_cast<Animation<T>*>(anim -> getAnimation()));
  }

  /// Map from pointers to variables to the corresponding animations.
  // This should be fine -- I'm just using void* as labels for the variables,
  // not anything else.
  typedef std::map<const void*, BaseAnimationDataPtr> Animations;

  /// The active animations.
  Animations  animations_;
  /// Keep track of time.
  Timer       time_;
};

/// This is a wrapper structure that helps with animating discrete types.
template <class T>
struct DiscreteAnimated {
  /// Type of the variable being animated.
  typedef T elem_type;
  /// Type to be used for parameters.
  typedef typename boost::call_traits<T>::param_type param_type;

  /// Empty constructor.
  DiscreteAnimated() : progress(1) {}
  /// Constructor with initialization.
  DiscreteAnimated(param_type val) : initial(val), target(val), progress(1) {}

  /// Check whether this variable is animated.
  bool isAnimated(const Animator& animator)
    { return animator.isAnimated(&progress); }
  
  /// Stop any animation for this variable.
  void deleteAnimation(Animator& animator) {
    animator.deleteAnimation(&progress);
    progress = 1;
  }

  /// Assignment.
  // XXX does this override the operator= for DiscreteAnimated
  DiscreteAnimated<T>& operator=(param_type value)
    { initial = target = value; progress = 1; return *this; }

  /// Initial value of the variable.
  elem_type   initial;
  /// Target value -- or current if it's not animated.
  elem_type   target;
  /// Progress.
  float       progress;
};

#endif
