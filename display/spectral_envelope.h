/** @file spectral_envelope.h
 *  @brief Define a spectral envelope display.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef SPECTRAL_ENVELOPE_H_
#define SPECTRAL_ENVELOPE_H_

#include <vector>

#include "animation/animator.h"
#include "display/axes.h"
#include "display/base_sdl_display.h"
#include "glutils/gl_incs.h"
#include "utils/misc.h"

/// Spectral envelope display.
class SpectralEnvelope : public BaseSdlDisplay {
 public:
  SpectralEnvelope() : n_points_(500) {}

  /// Implement the draw function.
  virtual void draw();

  /// Handle some events.
  virtual bool handleEvent(SDL_Event* event);

  /// Initialize the spectral envelope display.
  virtual int init();
  /// Clean up.
  virtual void done();

  /// Update the settings.
  virtual void updateProperties();

 private:
  unsigned                n_points_;
  Animator                animator_;
  Axes                    axes_;
  bool                    fill_;
  GlColor4                fill_color_;
};

#endif
