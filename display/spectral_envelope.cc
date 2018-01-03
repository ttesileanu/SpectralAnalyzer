#include "display/spectral_envelope.h"

#include <algorithm>
#include <complex>

#include "animation/standard_easing.h"
#include "glutils/color.h"
#include "glutils/geometry.h"
#include "glutils/vbo.h"
#include "input/base_input.h"
#include "processor/fft.h"
#include "processor/grabber.h"
#include "glutils/gl_incs.h"
#include "utils/logging.h"

void SpectralEnvelope::draw()
{
  // update the state of the animations
  animator_.update();
  axes_.updateAnimations();

  // get the data from the fft module
  FftProcessor::Output fft_output = boost::any_cast<FftProcessor::Output>
    (inputs_["fft"] -> getOutput());

  const Complex* data = fft_output -> fft;

  unsigned sz = fft_output -> size;
  unsigned sz2 = sz / 2;

  glDisable(GL_TEXTURE_2D);

  axes_.draw();

  // get access to the VBO
  std::vector<GlVertex2> points;

  unsigned n = n_points_;
  if (n > sz2) // no point in drawing more points than we have
    n = sz2;

  Grabber::Details raw_details = boost::any_cast<Grabber::Details>
    (inputs_["raw"] -> getDetails());

  float min_freq = (float)(raw_details -> samplingFrequency) / sz;
  const Rectangle& range = axes_.getRange();

  if (fill_) {
    Rectangle r = axes_.getRange(true);
    
    // XXX ugh, code repetition!
    for (unsigned i = 0; i < n; ++i) {
      // XXX this needs to be smarter
      float freq = range.start.x+(range.end.x - range.start.x)*(float)i/(n - 1);
      if (freq < min_freq)
        continue;

      unsigned idx = (freq - min_freq) / min_freq;
      if (idx >= sz2)
        continue;

      GlVertex2 p(freq, std::abs(data[idx]));
      GlVertex2 p_2(freq, r.start.y);

      points.push_back(axes_.graphToScreen(axes_.getClipped(p_2)));
      points.push_back(axes_.graphToScreen(axes_.getClipped(p)));
    }

    setGlColor(fill_color_);

    // send the data to OpenGL
    vbo_ -> draw(points, GL_QUAD_STRIP);
    points.clear();
  }

  for (unsigned i = 0; i < n; ++i) {
    // XXX this needs to be smarter
    float freq = range.start.x + (range.end.x - range.start.x)*(float)i/(n - 1);
    if (freq < min_freq)
      continue;

    unsigned idx = (freq - min_freq) / min_freq;
    if (idx >= sz2)
      continue;

    GlVertex2 p(freq, std::abs(data[idx]));

    points.push_back(axes_.graphToScreen(axes_.getClipped(p)));
  }

  // send the data to OpenGL
  vbo_ -> draw(points, GL_LINE_STRIP);
}

bool SpectralEnvelope::handleEvent(SDL_Event* event)
{
  bool handled = false;
  if (event -> type == SDL_KEYUP) {
    bool no_mods = (event -> key.keysym.mod == 0);
    bool shift = ((event -> key.keysym.mod & KMOD_SHIFT) != 0);
    bool just_shift = shift && ((event -> key.keysym.mod & (~KMOD_SHIFT)) == 0);
    switch (event -> key.keysym.sym) {
      case SDLK_a:
        if (no_mods) {
          axes_.flipVisibility();
          handled = true;
        }
        break;
      case SDLK_g:
        if (no_mods) {
          axes_.flipGridVisibility();
          handled = true;
        }
        break;
      case SDLK_l:
        if (no_mods) {
          Axes::ScalingType s = axes_.getScalingY();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setScalingY(s);
          handled = true;
        } else if (just_shift) {
          Axes::ScalingType s = axes_.getScalingX();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setScalingX(s);
          handled = true;
        }
        break;
      case SDLK_r:
        if (no_mods) {
          // XXX get size from sampling frequency
          Rectangle r (43, 0.03, 22050, 100);

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_EQUALS:
        if (no_mods) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y/0.75;
          r.end.y = r.end.y*0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        } else if (just_shift) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x/0.75;
          r.end.x = r.end.x*0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_MINUS:
        if (no_mods) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y*0.75;
          r.end.y = r.end.y/0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        } else if (just_shift) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x*0.75;
          r.end.x = r.end.x/0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_LEFT:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x/1.25;
          r.end.x = r.end.x/1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_RIGHT:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x*1.25;
          r.end.x = r.end.x*1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_DOWN:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y/1.25;
          r.end.y = r.end.y/1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_UP:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y*1.25;
          r.end.y = r.end.y*1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      default:;
    }
  }

  return handled;
}

int SpectralEnvelope::init()
{
  int err = BaseSdlDisplay::init();
  if (err)
    return err;

  n_points_ = properties_ -> get<unsigned>("npoints");

  // read filling properties
  fill_ = properties_ -> get<bool>("fill");
  fill_color_ = properties_ -> get<GlColor4>("fill_color");

  // set up non-configurable properties of the axes
  axes_.setType(Axes::BOX, "none");
  axes_.setTickType(Axes::BOTH, "none");

  // set up the configurable properties of the axes
  axes_.setProperties(&properties_ -> get_child("axes"));
  axes_.setClippingArea(axes_.getRange(), "none");
  axes_.setExtents(Rectangle(w_/40, h_/40, 39*w_/40, 39*h_/40), "none");
  axes_.setCrossing(GlVertex2(axes_.getRange().start.x, 
    axes_.getRange().start.y), "none");
  axes_.setTickOriginLinearX(axes_.getRange().start.x, "none");
  axes_.setTickOriginLinearY(axes_.getRange().start.y, "none");
  axes_.setTickOriginLogX(axes_.getRange().start.x, "none");
  axes_.setTickOriginLogY(axes_.getRange().start.y, "none");
/*  axes_.setTickSpacingX(axes_.getScalingX(), "none");
  axes_.setTickSpacingY(axes_.getScalingY(), "none");*/

  // set up the transitions
  axes_.setTransitionStore(transitions_);

  // set up the VBO
  const size_t vbo_size = 4*n_points_*sizeof(GlVertex2);
  vbo_.reset(new Vbo(vbo_size));

  return 0;
}

void SpectralEnvelope::done()
{
}

void SpectralEnvelope::updateProperties()
{
  properties_ -> put("npoints", n_points_);

  // write filling properties
  properties_ -> put("fill", fill_);
  properties_ -> put("fill_color", fill_color_);

  axes_.updateProperties();
}
