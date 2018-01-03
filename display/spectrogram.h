/** @file spectrogram.h
 *  @brief Define a spectrogram display.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef SPECTROGRAM_H_
#define SPECTROGRAM_H_

#include <vector>

#include <boost/scoped_ptr.hpp>

#include "animation/animator.h"
#include "display/axes.h"
#include "display/base_sdl_display.h"
#include "glutils/color.h"
#include "glutils/fbo.h"
#include "glutils/gl_incs.h"
#include "glutils/vbo.h"

/// Spectrogram display.
class Spectrogram : public BaseSdlDisplay {
 public:
  Spectrogram() : crt_fbo_(0), shift_(2) {}

  /// Implement the draw function.
  virtual void draw();

  /// Handle some events.
  virtual bool handleEvent(SDL_Event* event);

  /// Initialize the spectrogram display.
  virtual int init();
  /// Clean up.
  virtual void done();

  /// Update the settings.
  virtual void updateProperties();

  /// Reset axes.
  void resetAxes();

  /// Generate the palette.
  void makePalette(const std::string& s);

 private:
  void makePalette_(const std::string& s);
  GlColor4 getColor_(float a);
  void scroll_();

  Animator                animator_;
  boost::scoped_ptr<Vbo>  vbo_;
  boost::scoped_ptr<Fbo>  fbos_[2];
  int                     crt_fbo_;
  Axes                    axes_;
  unsigned                shift_;
  std::vector<GlColor4>   palette_;
};

#endif
