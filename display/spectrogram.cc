#include "display/spectrogram.h"

#include <algorithm>
#include <complex>

#include <boost/lexical_cast.hpp>

#include "animation/standard_easing.h"
#include "glutils/geometry.h"
#include "input/base_input.h"
#include "processor/fft.h"
#include "processor/grabber.h"
#include "glutils/gl_incs.h"
#include "utils/logging.h"
#include "utils/misc.h"

void Spectrogram::draw()
{
  // update the state of animations
  animator_.update();
  axes_.updateAnimations();

  // scroll the screen
  scroll_();

  // draw in the correct FBO
  Fbo::push();
  fbos_[crt_fbo_] -> bind();

  // get the data from the fft module
  FftProcessor::Output fft_output = boost::any_cast<FftProcessor::Output>
    (inputs_["fft"] -> getOutput());

  const Complex* data = fft_output -> fft;

  unsigned sz = fft_output -> size;
  int sz2 = sz / 2;

  glDisable(GL_TEXTURE_2D);

  // make a buffer to send to the VBO
  std::vector<GlColoredVertex2> points;

  const Rectangle& extents = axes_.getExtents(true);

  Grabber::Details raw_details = boost::any_cast<Grabber::Details>
    (inputs_["raw"] -> getDetails());

  float min_freq = (float)(raw_details -> samplingFrequency) / sz;
  for (unsigned i = extents.start.x; i < extents.end.x; ++i) {
    const float freq = axes_.screenToGraph(GlVertex2(i, extents.start.y)).x;
    
    int idx = (freq - min_freq) / min_freq;
    GlColor4 color(0, 0, 0);
    if (idx >= 0 || idx < sz2) {
      const float amplitude = std::abs(data[idx]);
      GlVertex2 p = axes_.graphToScreen(GlVertex2(freq, amplitude));
      color = getColor_(p.y);
    }

    GlColoredVertex2 vertex1(w_ - shift_, i, color);
    GlColoredVertex2 vertex2(w_, i, color);
    GlColoredVertex2 vertex3(w_, i + 1, color);
    GlColoredVertex2 vertex4(w_ - shift_, i + 1, color);

    points.push_back(vertex1);
    points.push_back(vertex2);
    points.push_back(vertex3);
    points.push_back(vertex4);
  }

  vbo_ -> draw(points, GL_QUADS);

  // go back to the display FBO, and transfer it to screen
  Fbo::pop();

  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  fbos_[crt_fbo_] -> getTexture() -> bind();
  setGlColor(GlColor4(1, 1, 1));

  std::vector<GlVertexTex2> points_tex;
  points_tex.push_back(GlVertexTex2(0, 0, 0, 0));
  points_tex.push_back(GlVertexTex2(w_, 0, 1, 0));
  points_tex.push_back(GlVertexTex2(w_, h_, 1, 1));
  points_tex.push_back(GlVertexTex2(0, h_, 0, 1));

  // select the texture
  glClientActiveTexture(GL_TEXTURE0);

  // send the data to OpenGL
  vbo_ -> draw(points_tex, GL_QUADS);
  glDisable(GL_TEXTURE_2D);
}

bool Spectrogram::handleEvent(SDL_Event* event)
{
  bool handled = false;
  if (event -> type == SDL_KEYUP) {
    bool no_mods = (event -> key.keysym.mod == 0);
    bool shift = ((event -> key.keysym.mod & KMOD_SHIFT) != 0);
    bool just_shift = shift && ((event -> key.keysym.mod & (~KMOD_SHIFT)) == 0);
    switch (event -> key.keysym.sym) {
      case SDLK_l:
        if (no_mods) {
          Axes::ScalingType s = axes_.getScalingX();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setScalingX(s);
          handled = true;
        } else if (just_shift) {
          Axes::ScalingType s = axes_.getScalingY();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setScalingY(s);
          handled = true;
        }
        break;
      case SDLK_r:
        if (no_mods) {
          resetAxes();
          handled = true;
        }
        break;
      case SDLK_EQUALS:
        if (just_shift) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y/0.75;
          r.end.y = r.end.y*0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        } else if (no_mods) {
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
        if (just_shift) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y*0.75;
          r.end.y = r.end.y/0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        } else if (no_mods) {
          // XXX make configurable zoom factor
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x*0.75;
          r.end.x = r.end.x/0.75;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_DOWN:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x/1.25;
          r.end.x = r.end.x/1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_UP:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.x = r.start.x*1.25;
          r.end.x = r.end.x*1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_LEFT:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          r.start.y = r.start.y/1.25;
          r.end.y = r.end.y/1.25;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
       break;
      case SDLK_RIGHT:
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

int Spectrogram::init()
{
  int err = BaseSdlDisplay::init();
  if (err)
    return err;

  // set up the palette
  makePalette(properties_ -> get<std::string>("palette"));

  // set up non-configurable properties of the axes
  axes_.setVisibility(false, "none");

  // set up the configurable properties of the axes
  axes_.setProperties(&properties_ -> get_child("axes"));
  axes_.setClippingArea(axes_.getRange(), "none");
  axes_.setExtents(Rectangle(0, 0, h_, 1), "none");

  // set up the transitions
  axes_.setTransitionStore(transitions_);

  // set up the VBO
  const size_t vbo_size = 4*2048*sizeof(GlVertexTex2);
  vbo_.reset(new Vbo(vbo_size));

  // set up the FBOs
  // XXX not sure this is the proper size...
  for (int i = 0; i < 2; ++i) {
    fbos_[i].reset(new Fbo(w_, h_));
    glClear(GL_COLOR_BUFFER_BIT);
  }

  return 0;
}

void Spectrogram::done()
{
}

void Spectrogram::updateProperties()
{
  axes_.updateProperties();
}

GlColor4 Spectrogram::getColor_(float a)
{
  if (a < 0)
    a = 0;
  if (a > 1)
    a = 1;

  unsigned sz = palette_.size();
  unsigned idx = a*sz;
  if (idx >= sz)
    idx = sz - 1;

  return palette_[idx];
}

void Spectrogram::scroll_()
{
  Fbo::push();

  int old_fbo_ = crt_fbo_;
  crt_fbo_ = (1 - crt_fbo_);

  glEnable(GL_TEXTURE_2D);
  fbos_[crt_fbo_] -> bind();
  fbos_[old_fbo_] -> getTexture() -> bind();
  setGlColor(GlColor4(1, 1, 1));

  // fill the VBO
  float shift_tex = (float)(shift_) / w_;

  std::vector<GlVertexTex2> points_tex;
  points_tex.push_back(GlVertexTex2(0, 0, shift_tex, 0));
  points_tex.push_back(GlVertexTex2(w_ - shift_, 0, 1, 0));
  points_tex.push_back(GlVertexTex2(w_ - shift_, h_, 1, 1));
  points_tex.push_back(GlVertexTex2(0, h_, shift_tex, 1));

  // select the texture
  glClientActiveTexture(GL_TEXTURE0);

  // send the data to OpenGL
  vbo_ -> draw(points_tex, GL_QUADS);
  glDisable(GL_TEXTURE_2D);

  Fbo::pop();
}

void Spectrogram::resetAxes()
{
  // XXX get size from sampling frequency
  Rectangle r (43, 0.03, 22050, 100);

  axes_.setRange(r);
  axes_.setClippingArea(r);
}

void Spectrogram::makePalette(const std::string& s)
{
  if (s == "grayscale")
    makePalette_("rgb 0:(0,0,0,1) 1:(1,1,1,1)");
  else if (s == "thermal")
    makePalette_("hls 0:(1,0,0.3,1) 0.9:(0,0.6,1,1) 1:(0,1,0,1)");
  else
    makePalette_(s);
}

static GlColor4 hlsToRgb(const GlColor4& col)
{
  float hue = col.r;
  float lum = col.g;
  float sat = col.b;

  GlColor4 res;
  res.a = col.a;

  if (hue < 1.0/6) {
    res.r = 1;
    res.g = hue*6;
    res.b = 0;
  } else if (hue < 2.0/6) {
    res.r = (2.0/6 - hue)*6;
    res.g = 1;
    res.b = 0;
  } else if (hue < 3.0/6) {
    res.r = 0;
    res.g = 1;
    res.b = (hue - 2.0/6)*6;
  } else if (hue < 4.0/6) {
    res.r = 0;
    res.g = (4.0/6 - hue)*6;
    res.b = 1;
  } else if (hue < 5.0/6) {
    res.r = (hue - 4.0/6)*6;
    res.g = 0;
    res.b = 1;
  } else {
    res.r = 1;
    res.g = 0;
    res.b = (1 - hue)*6;
  }

  float min = 0.5 - sat/2;
  float max = 0.5 + sat/2;

  res.r = min + res.r*(max - min);
  res.g = min + res.g*(max - min);
  res.b = min + res.b*(max - min);

  if (lum <= 0.5) {
    float f = 2*lum;
    res.r = res.r*f;
    res.g = res.g*f;
    res.b = res.b*f;
  } else {
    float f = 2*lum - 1;
    res.r = res.r + (1 - res.r)*f;
    res.g = res.g + (1 - res.g)*f;
    res.b = res.b + (1 - res.b)*f;
  }

  return res;
}

void Spectrogram::makePalette_(const std::string& s)
{
  if (s.length() < 4)
    throw Exception("Bad palette string: " + s);

  std::string type = s.substr(0, 3);
  if (type != "hls" && type != "rgb")
    throw Exception("Bad palette string: " + s);

  std::vector<std::string> points = splitString(s.substr(4));
  const size_t n_points = points.size();

  const size_t n = 256;
  palette_.resize(n, GlColor4(0, 0, 0));

  if (n_points > 0) {
    for (size_t i = 0; i < n_points - 1; ++i) {
      std::string s1 = points[i];
      std::string s2 = points[i + 1];

      size_t p1 = s1.find(':');
      size_t p2 = s2.find(':');
      if (p1 == std::string::npos || p2 == std::string::npos)
        continue;

      size_t idx1 = boost::lexical_cast<float>(s1.substr(0, p1))*n;
      size_t idx2 = boost::lexical_cast<float>(s2.substr(0, p2))*n;

      std::string scol1 = s1.substr(p1 + 1);
      std::string scol2 = s2.substr(p2 + 1);
      if (scol1.length() < 3 || scol1[0] != '(' ||
          scol1[scol1.length() - 1] != ')')
        throw Exception("Bad palette string: " + s);
      if (scol2.length() < 3 || scol2[0] != '(' ||
          scol2[scol2.length() - 1] != ')')
        throw Exception("Bad palette string: " + s);

      scol1 = scol1.substr(1, scol1.length() - 2);
      scol2 = scol2.substr(1, scol2.length() - 2);
      GlColor4 col1 = boost::lexical_cast<GlColor4>(scol1);
      GlColor4 col2 = boost::lexical_cast<GlColor4>(scol2);

      for (size_t j = idx1; j < idx2; ++j) {
        float alpha = (float)(j - idx1) / (idx2 - idx1);
        GlColor4 col = (1 - alpha)*col1 + alpha*col2;

        if (type == "hls")
          col = hlsToRgb(col);

        palette_[j] = col;
      }
    }
  }
}
