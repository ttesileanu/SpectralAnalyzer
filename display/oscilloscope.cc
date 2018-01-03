#include "display/oscilloscope.h"

#include <algorithm>

#include "animation/standard_easing.h"
#include "animation/transition_store.h"
#include "glutils/color.h"
#include "glutils/geometry.h"
#include "input/base_input.h"
#include "processor/grabber.h"
#include "glutils/gl_incs.h"
#include "utils/exception.h"
#include "utils/logging.h"

void Oscilloscope::draw()
{
  // update the state of the animations
  animator_.update();
  axes_.updateAnimations();

  // get the data from the input module
  Grabber::Output pdata = boost::any_cast<Grabber::Output>
    (inputs_["raw"] -> getOutput());
  const std::vector<float>& data = *pdata;

  float shift = 0;
  unsigned sz2 = data.size()/2;
  if (max_shift_ > 0) {
    // find the minimal shift that places a zero in the center of the display
    // find a zero through which the function is increasing
    unsigned dist = std::min((unsigned)(max_shift_*sz2), sz2);
    for (unsigned i = 1; i < dist; ++i) {
      if (data[sz2 + i] >= 0 && data[sz2 + i - 1] <= 0) {
        shift = i;
        break;
      } else if (data[sz2 - i] <= 0 && data[sz2 - i + 1] >= 0) {
        shift = -(float)i;
        break;
      }
    }
  }

  glDisable(GL_TEXTURE_2D);

  // draw the axes
  axes_.draw();

  float fin_points = ((style_.target == S_POINTS || style_.target == S_BOTH)?
    1:0);
  float fin_lines = ((style_.target == S_LINES || style_.target == S_BOTH)?1:0);

  float alpha_points, alpha_lines;
  if (style_.isAnimated(animator_)) {
    float ini_points, ini_lines;
    ini_points = ((style_.initial == S_POINTS || style_.initial == S_BOTH)?1:0);
    ini_lines = ((style_.initial == S_LINES || style_.initial == S_BOTH)?1:0);

    alpha_points = ini_points + (fin_points - ini_points)*style_.progress;
    alpha_lines = ini_lines + (fin_lines - ini_lines)*style_.progress;
  } else {
    alpha_points = fin_points;
    alpha_lines = fin_lines;
  }

  drawLines_(data, alpha_lines, shift);
  drawPoints_(data, alpha_points, shift);
}

bool Oscilloscope::handleEvent(SDL_Event* event)
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
      case SDLK_f:
        if (no_mods) {
          cycleStyle();
          handled = true;
        } else if (just_shift) {
          revCycleStyle();
          handled = true;
        }
        break;
      case SDLK_g:
        if (no_mods) {
          axes_.flipGridVisibility();
          handled = true;
        }
        break;
      case SDLK_s:
        if (no_mods) {
          flipZeroFixState();
          handled = true;
        }
        break;
      case SDLK_r:
        if (no_mods) {
          Rectangle r (0, -1, 1, 1);

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_EQUALS:
        if (no_mods) {
          // XXX configurable zoom factor!
          Rectangle r = axes_.getRange();
          float mid_y = (r.start.y + r.end.y) / 2;
          float len_y_2 = (r.end.y - r.start.y) / 2;
          r.start.y = mid_y - 0.9*len_y_2;
          r.end.y = mid_y + 0.9*len_y_2;

          axes_.setRange(r);
          axes_.setClippingArea(r);
          handled = true;
        }
        break;
      case SDLK_MINUS:
        if (no_mods) {
          Rectangle r = axes_.getRange();
          float mid_y = (r.start.y + r.end.y) / 2;
          float len_y_2 = (r.end.y - r.start.y) / 2;
          r.start.y = mid_y - len_y_2/0.9;
          r.end.y = mid_y + len_y_2/0.9;

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

int Oscilloscope::init()
{
  int err = BaseSdlDisplay::init();
  if (err)
    return err;

  n_points_ = properties_ -> get<unsigned>("npoints");
  max_shift_limit_ = properties_ -> get<float>("maxshift");
  setZeroFixState(properties_ -> get<bool>("zerofix"));
  setStyle(properties_ -> get<std::string>("style"), "none");

  // set up non-configurable properties of the axes
  axes_.setType(Axes::CROSS, "none");
  axes_.setTickType(Axes::BOTH, "none");
  axes_.setTicksTwoSided(true, "none");
  axes_.setBoxVisibility(false, "none");
  axes_.setExtents(Rectangle(0, 0, w_, h_), "none");
  axes_.setCrossing(GlVertex2(0.5, 0), "none");
  axes_.setScalingX(Axes::LINEAR, "none");
  axes_.setScalingY(Axes::LINEAR, "none");
  axes_.setTickOriginLinearX(0.5, "none");
  axes_.setTickOriginLinearY(0, "none");
  axes_.setTickSpacingX(Axes::LINEAR, "none");
  axes_.setTickSpacingY(Axes::LINEAR, "none");
  // the vertical component will be modified by the ini file
  axes_.setRange(Rectangle(0, -1, 1, 1), "none");

  // set up the configurable properties of the axes
  axes_.setProperties(&properties_ -> get_child("axes"));
  axes_.setClippingArea(axes_.getRange(), "none");

  // set up the transitions
  axes_.setTransitionStore(transitions_);

  // create the VBO
  const size_t vbo_size = 8*n_points_*sizeof(GlVertex2);
  vbo_.reset(new Vbo(vbo_size));

  return 0;
}

void Oscilloscope::done()
{
}

void Oscilloscope::setStyle(const std::string& s, const std::string& trans)
{
  if (s == "lines")
    setStyle(S_LINES, trans);
  else if (s == "points")
    setStyle(S_POINTS, trans);
  else if (s == "both")
    setStyle(S_POINTS, trans);
  else
    throw Exception("Unknown oscilloscope style: " + s + ".");
}

void Oscilloscope::setStyle(Style s, const std::string& trans)
{
  style_.initial = style_.target;
  style_.target = s;
  animator_.doTransition(&style_.progress, 0, 1,
    getTransition_("type_change", trans));
}

void Oscilloscope::cycleStyle(const std::string& trans)
{
  switch (getStyle()) {
    case S_POINTS:
      setStyle(S_LINES, trans);
      break;
    case S_LINES:
      setStyle(S_BOTH, trans);
      break;
    case S_BOTH:
      setStyle(S_POINTS, trans);
      break;
  }
}

void Oscilloscope::revCycleStyle(const std::string& trans)
{
  switch (getStyle()) {
    case S_POINTS:
      setStyle(S_BOTH, trans);
      break;
    case S_LINES:
      setStyle(S_POINTS, trans);
      break;
    case S_BOTH:
      setStyle(S_LINES, trans);
      break;
  }
}

std::string Oscilloscope::getStyleString() const
{
  switch (getStyle()) {
    case S_LINES:
      return "lines";
    case S_POINTS:
      return "points";
    case S_BOTH:
      return "both";
  }

  return "";
}

void Oscilloscope::updateProperties()
{
  properties_ -> put("npoints", n_points_);
  properties_ -> put("maxshift", max_shift_limit_);
  properties_ -> put("zerofix", getZeroFixState());
  properties_ -> put("style", getStyleString());

  axes_.updateProperties();
}

static std::pair<float, BaseEasingPtr> trivial_trans(0, BaseEasingPtr());

const std::pair<float, BaseEasingPtr>& Oscilloscope::getTransition_
    (const std::string& name, const std::string& trans)
{
  if (trans == "none" || !transitions_) {
    return trivial_trans;
  } else if (trans.empty()) {
    return transitions_ -> get(name);
  } else {
    return transitions_ -> get(trans);
  }
}

void Oscilloscope::drawLines_(const std::vector<float>& data, float alpha,
    float shift)
{
  unsigned sz = data.size();

  // data for the VBO
  std::vector<GlVertex2> points;

  unsigned n = n_points_;
  if (n > sz) // no point in drawing more points than we have
    n = sz;

  float dt = (float)sz/n;
  for (unsigned i = 0; i < n; ++i) {
    const float t = i*dt + shift;

    if (t < 0)
      continue;
    if (t > sz)
      break;

    GlVertex2 p((float)i/(n - 1), data[t]);

    points.push_back(axes_.graphToScreen(axes_.getClipped(p)));
  }

  // XXX make this configurable
  setGlColor(GlColor4(alpha, alpha, alpha, alpha));

  // send the data to OpenGL
  vbo_ -> draw(points, GL_LINE_STRIP);
}

void Oscilloscope::drawPoints_(const std::vector<float>& data, float alpha,
    float shift)
{
  // XXX UGH! so much code duplication!

  unsigned sz = data.size();

  // data for the VBO
  std::vector<GlVertex2> points;

  unsigned n = n_points_;
  if (n > sz) // no point in drawing more points than we have
    n = sz;

  float dt = (float)sz/n;

  // XXX make this configurable!
  GlVertex2 cr_horiz(3, 0);
  GlVertex2 cr_vert(0, 3);

  for (unsigned i = 0; i < n; ++i) {
    const float t = i*dt + shift;

    if (t < 0)
      continue;
    if (t > sz)
      break;

    GlVertex2 p((float)i/(n - 1), data[t]);

    points.push_back(axes_.graphToScreen(axes_.getClipped(p)) - cr_horiz);
    points.push_back(axes_.graphToScreen(axes_.getClipped(p)) - cr_vert);
    points.push_back(axes_.graphToScreen(axes_.getClipped(p)) + cr_horiz);
    points.push_back(axes_.graphToScreen(axes_.getClipped(p)) + cr_vert);
  }

  // XXX make this configurable
  setGlColor(GlColor4(alpha, alpha, alpha, alpha));

  // send the data to OpenGL
  vbo_ -> draw(points, GL_QUADS);
}
