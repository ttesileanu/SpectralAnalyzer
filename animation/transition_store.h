/** @file transition_store.h
 *  @brief Defines a class that stores various transition patterns, including
 *         their timing and easing functions.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef TRANSITION_STORE_H_
#define TRANSITION_STORE_H_

#include <map>
#include <string>
#include <utility>

#include "utils/exception.h"
#include "utils/forward_defs.h"
#include "utils/properties.h"

/// This stores transition patterns associated with names.
class TransitionStore {
 public:
  /// A single transition: length of time, and easing function.
  typedef std::pair<float, BaseEasingPtr> Transition;

  /// Empty constructor.
  TransitionStore() : properties_(0) {}

  /// Add a transition.
  void add(const std::string& name, float len, const BaseEasingPtr& easing)
    { transitions_[name] = std::make_pair(len, easing); }

  /// Get a transition.
  const Transition& get(const std::string& name) const {
    Transitions::const_iterator i = transitions_.find(name);
    if (i == transitions_.end())
      throw Exception("Transition '" + name + "' cannot be found in transition "
        "store (::get).");
    return i -> second;
  }

  /// Get a transition's duration.
  float getDuration(const std::string& name) const
    { return get(name).first; }

  /// Get a transition's easing function.
  const BaseEasingPtr& getEasing(const std::string& name) const
    { return get(name).second; }

  /// Get access to settings. This also sets all the properties.
  void setProperties(Properties* props);

  /// Update the settings -- does nothing for now.
  void updateProperties() {}

 private:
  /// A map from strings to transitions.
  typedef std::map<std::string, Transition> Transitions;

  Transitions       transitions_;
  Properties*       properties_;
};

#endif
