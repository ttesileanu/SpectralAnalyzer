/** @file oscilloscope.h
 *  @brief Define an oscilloscope display.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef OSCILLOSCOPE_H_
#define OSCILLOSCOPE_H_

#include <vector>

#include "animation/animator.h"
#include "display/axes.h"
#include "display/base_sdl_display.h"
#include "glutils/gl_incs.h"
#include "utils/misc.h"

/// Oscilloscope display.
class Oscilloscope : public BaseSdlDisplay {
 public:
  /// The style of the display -- lines, points, or both.
  enum Style { S_POINTS, S_LINES, S_BOTH };

  /// Constructor.
  Oscilloscope() : n_points_(500), max_shift_limit_(0.4),
    max_shift_(max_shift_limit_), zero_fix_transition_time_(0.4),
    style_(S_LINES) {}

  /// Implement the draw function.
  virtual void draw();

  /// Handle some events.
  virtual bool handleEvent(SDL_Event* event);

  /// Initialize the oscilloscope display.
  virtual int init();
  /// Clean up.
  virtual void done();

  /// Set whether to do zero-fixing or not.
  void setZeroFixState(bool b) { max_shift_ = b?max_shift_limit_:0; }
  
  /// Flip zero-fixing state.
  void flipZeroFixState() { setZeroFixState(!getZeroFixState()); }

  /// Find out whether we're doing zero-fixing.
  bool getZeroFixState() const { return (max_shift_ > 1e-3); }

  /// Set the style of the display ("points", "lines", or "both").
  void setStyle(const std::string& s, const std::string& trans = std::string());

  /// Set the style of the display.
  void setStyle(Style s, const std::string& trans = std::string());

  /// Cycle to the next display style.
  void cycleStyle(const std::string& trans = std::string());

  /// Cycle to the previous display style.
  void revCycleStyle(const std::string& trans = std::string());

  /// Get the current display style.
  Style getStyle() const { return style_.target; }

  /// Get the current display style as a string.
  std::string getStyleString() const;

  /// Update the settings.
  virtual void updateProperties();

 private:
  const std::pair<float, BaseEasingPtr>& getTransition_
      (const std::string& name, const std::string& trans);
  void drawPoints_(const std::vector<float>& data, float alpha, float shift);
  void drawLines_(const std::vector<float>& data, float alpha, float shift);

  unsigned                n_points_;
  Animator                animator_;
  Axes                    axes_;
  float                   max_shift_limit_;
  float                   max_shift_;
  float                   zero_fix_transition_time_;
  DiscreteAnimated<Style> style_;
};

#endif
