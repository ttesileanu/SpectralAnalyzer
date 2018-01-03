/** @file base_display.h
 *  @brief Defines the interface for a display module.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef BASE_DISPLAY_H_
#define BASE_DISPLAY_H_

#include <boost/scoped_ptr.hpp>

#include "processor/base_processor.h"
#include "glutils/vbo.h"
#include "utils/forward_defs.h"
#include "utils/properties.h"

/// This is the interface required of all display modules.
class BaseDisplay : public BaseProcessor {
 public:
  /// Virtual destructor, for inheritance.
  virtual ~BaseDisplay() {}

  /** @brief Perform the drawing, using data from the modules that are given
   *  to the processor as input modules.
   *
   *  This is the function that must be overridden by descendants.
   */
  virtual void draw() = 0;

  /// Let the display know what size of a window it has to draw in.
  void resize(float w, float h) { w_ = w; h_ = h; }

  /** @brief Initialize the module, if necessary.
   *
   *  Should return 0 if successful, nonzero otherwise. By default, this does
   *  nothing. Override to do appropriate initialization.
   */
  virtual int init() {
    return (properties_)?0:1;
  };

  /** @brief Clean up after the module, if necessary.
   *
   *  Does nothing by default; override if you need some clean up.
   */
  virtual void done() {};

  /// Give the module access to its settings.
  void setProperties(Properties* props) { properties_ = props; }

  /// Update the settings. Descendants should implement this. @see setProperties
  virtual void updateProperties() {}

  /// Give access to a transition store.
  void setTransitionStore(const TransitionStorePtr& t)
    { transitions_ = t; }

 protected:
  BaseDisplay() : properties_(0), w_(640), h_(480) {}

  // this kind of processor has no output
  int execute() { markValid(); return 0; }

  boost::any getOutput_() const { return boost::any(); }

  /// Access to a transition store.
  TransitionStorePtr    transitions_;
  Properties*           properties_;
  float                 w_;
  float                 h_;
  /// A VBO for the display.
  boost::scoped_ptr<Vbo>  vbo_;
};

#endif
