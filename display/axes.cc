#include "axes.h"

#include <algorithm>

#include <cmath>

#include "animation/transition_store.h"
#include "utils/exception.h"

namespace {

  enum Direction { HORIZONTAL, VERTICAL};

} // unnamed namespace

static const float eps = 1e-6;

static void drawAxis(const Rectangle& axis, std::vector<GlVertex2>& points)
{
  points.push_back(axis.start);
  points.push_back(axis.end);
}

static void drawGrid(const std::vector<float>& positions, Direction direction,
  const Rectangle& axes_box, std::vector<GlVertex2>& points)
{
  size_t n = positions.size();
  for (size_t i = 0; i < n; ++i) {
    GlVertex2 p1, p2;

    if (direction == HORIZONTAL) {
      p1.x = positions[i];
      p2.x = positions[i];
      p1.y = axes_box.start.y;
      p2.y = axes_box.end.y;
    } else {
      p1.x = axes_box.start.x;
      p2.x = axes_box.end.x;
      p1.y = positions[i];
      p2.y = positions[i];
    }
    points.push_back(p1);
    points.push_back(p2);
  }
}

// "up" points ccw from axis
static void drawTicks(const Rectangle& axis,
    const std::vector<float>& positions, float size_up, float size_down,
    std::vector<GlVertex2>& points)
{
  Direction dir;
  GlVertex2 tick_up, tick_down;
  // XXX I'm assuming the axis is aligned with either the horizontal or the
  // vertical!
  if (std::abs(axis.start.x - axis.end.x) < eps) {
    dir = VERTICAL;

    tick_up = GlVertex2(-size_up, 0);
    tick_down = GlVertex2(size_down, 0);
  } else {
    dir = HORIZONTAL;

    tick_up = GlVertex2(0, size_up);
    tick_down = GlVertex2(0, -size_down);
  }

  size_t n = positions.size();
  for (size_t i = 0; i < n; ++i) {
    GlVertex2 point;

    if (dir == HORIZONTAL) {
      point.x = positions[i];
      point.y = axis.start.y;
    } else {
      point.x = axis.start.x;
      point.y = positions[i];
    }
 
    points.push_back(point + tick_down);
    points.push_back(point + tick_up);
  }
}

static void drawBox(const Rectangle& box, Vbo* vbo)
{
  std::vector<GlVertex2> points;

  points.push_back(box.start);
  points.push_back(GlVertex2(box.end.x, box.start.y));
  points.push_back(box.end);
  points.push_back(GlVertex2(box.start.x, box.end.y));
  points.push_back(box.start);

  vbo -> draw(points, GL_LINE_STRIP);
}

std::vector<float> Axes::calculateTicks_(const Rectangle& axis_graph,
    const TicksInfo& info)
{
  Direction dir;
  // XXX I'm assuming the axis is aligned with either the horizontal or the
  // vertical!
  if (std::abs(axis_graph.start.x - axis_graph.end.x) < eps)
    dir = VERTICAL;
  else
    dir = HORIZONTAL;

  float start, end;
  if (dir == HORIZONTAL) {
    start = axis_graph.start.x;
    end = axis_graph.end.x;
  } else {
    start = axis_graph.start.y;
    end = axis_graph.end.y;
  }

  int k_min_linear = std::floor((start-info.origin_linear)/info.spacing_linear);
  int k_max_linear = std::ceil((end-info.origin_linear)/info.spacing_linear);

  bool log_good = true;

  float log_spacing;
  int k_min_log;
  int k_max_log;

  if ((start > 0 && end > 0) || (start < 0 && end < 0 && info.origin_log < 0)) {
    log_spacing = std::log(info.spacing_log);
    k_min_log = std::floor(std::log(start/info.origin_log) / log_spacing);
    k_max_log = std::ceil(std::log(end/info.origin_log) / log_spacing);
  } else {
    log_good = false;
    k_min_log = k_min_linear;
    k_max_log = k_max_linear;
  }

  int k_min = std::min(k_min_linear, k_min_log);
  int k_max = std::max(k_max_linear, k_max_log);

  float logginess = (info.spacing_type.target == LOG)?
      info.spacing_type.progress:(1 - info.spacing_type.progress);

  std::vector<float> res;
  for (int k = k_min; k <= k_max; ++k) {
    // XXX it's not nice that I calculate both the linear and log ticks all
    // the time...
    float lin_tick = info.origin_linear + info.spacing_linear*k;
    float tick;

    if (log_good) {
      float exponent = k*log_spacing;
      // XXX hard-coded, and weird... Is there a better solution?
      if (exponent > 128)
        exponent = 128;

      float log_tick = info.origin_log*std::exp(exponent);

      tick = (1 - logginess)*lin_tick + logginess*log_tick;
    } else {
      tick = lin_tick;
    }

    if (tick < start)
      continue;
    if (tick > end)
      break;

    // I have the graph-space position of the tick... Where is it in screen
    // space?
    GlVertex2 graph_point;
    if (dir == HORIZONTAL) {
      graph_point.x = tick;
      graph_point.y = axis_graph.start.y;
    } else {
      graph_point.x = axis_graph.start.x;
      graph_point.y = tick;
    }
    GlVertex2 point = graphToScreen(graph_point);

    if (dir == HORIZONTAL)
      res.push_back(point.x);
    else
      res.push_back(point.y);
  }

  return res;
}

void Axes::draw()
{
  if (!vbo_) {
    const size_t vbo_size = 2048*sizeof(GlVertex2);
    vbo_.reset(new Vbo(vbo_size));
  }

  Type type = type_.target;
  double axis_opacity = visibility_;

  if (axis_opacity < eps && box_ < eps) // nothing to do
    return;

  // some values don't make sense if they're different for minor&major axes
  ticks_x_maj_.spacing_type = ticks_x_min_.spacing_type;
  ticks_y_maj_.spacing_type = ticks_y_min_.spacing_type;

  ticks_x_maj_.origin_linear = ticks_x_min_.origin_linear;
  ticks_y_maj_.origin_linear = ticks_y_min_.origin_linear;
  ticks_x_maj_.origin_log = ticks_x_min_.origin_log;
  ticks_y_maj_.origin_log = ticks_y_min_.origin_log;

  // XXX ideally many of these things would be precalculated and remembered
  // for the case (which is most common) when there is no animation

  // positions of axes on screen
  Rectangle x_axis;
  Rectangle y_axis;
  // axes ranges in graph space
  Rectangle x_axis_graph;
  Rectangle y_axis_graph;
  // crossing point of axes, in graph space
  GlVertex2 crossing_graph;
  float boxiness;
  if (type_.isAnimated(animator_)) {
    // animate this if we're transitioning between box and cross...
    GlVertex2 crossing_box = range_.start;

    boxiness = ((type_.target == BOX)?(type_.progress):(1 - type_.progress));

    // XXX hard coded!
    float pos = ((boxiness > 0.7)?1:(boxiness/0.7));
    crossing_graph = pos*crossing_box + (1 - pos)*crossing_;
  } else {
    boxiness = ((type_.target == BOX)?1:0);
    crossing_graph = ((type == BOX)?range_.start:crossing_);
  }
  // crossing point in screen-space
  GlVertex2 crossing = graphToScreen(crossing_graph);

  x_axis.start = GlVertex2(axes_box_.start.x, crossing.y);
  x_axis.end = GlVertex2(axes_box_.end.x, crossing.y);

  y_axis.start = GlVertex2(crossing.x, axes_box_.start.y);
  y_axis.end = GlVertex2(crossing.x, axes_box_.end.y);

  x_axis_graph.start = GlVertex2(range_.start.x, crossing_graph.y);
  x_axis_graph.end = GlVertex2(range_.end.x, crossing_graph.y);

  y_axis_graph.start = GlVertex2(crossing_graph.x, range_.start.y);
  y_axis_graph.end = GlVertex2(crossing_graph.x, range_.end.y);

  std::vector<float> maj_ticks_pos_x;
  std::vector<float> maj_ticks_pos_y;

  std::vector<GlVertex2> points;

  float grid_opacity = grid_*axis_opacity;
  if (grid_opacity >= eps) {
    maj_ticks_pos_x = calculateTicks_(x_axis_graph, ticks_x_maj_);
    maj_ticks_pos_y = calculateTicks_(y_axis_graph, ticks_y_maj_);

    setGlColor(cosmetics_.grid_color*grid_opacity);
    drawGrid(maj_ticks_pos_x, HORIZONTAL, axes_box_, points);
    drawGrid(maj_ticks_pos_y, VERTICAL, axes_box_, points);

    vbo_ -> draw(points, GL_LINES);
    points.clear();
  }

  setGlColor(cosmetics_.color*axis_opacity);
  if (axis_opacity >= eps) {
    drawAxis(x_axis, points);
    drawAxis(y_axis, points);
    // XXX hard-coded!
    if (boxiness >= 0.7) {
      vbo_ -> draw(points, GL_LINES);
      points.clear();

      float other_axes_opac = (boxiness - 0.7) / 0.3;
      setGlColor(cosmetics_.color*(axis_opacity*other_axes_opac));
      
      drawAxis(Rectangle(GlVertex2(axes_box_.start.x, axes_box_.end.y),
        axes_box_.end), points);
      drawAxis(Rectangle(GlVertex2(axes_box_.end.x, axes_box_.start.y),
        axes_box_.end), points);

      vbo_ -> draw(points, GL_LINES);
      points.clear();
    }
  }

  float two_sidedness = ticks_twosided_*(1 - boxiness);

  float min_tick_opacity = 0;
  float maj_tick_opacity = 0;
  if (!ticks_.isAnimated(animator_)) { // there's no animation at all...
    min_tick_opacity = (ticks_.target == MINOR || ticks_.target == BOTH)?1:0;
    maj_tick_opacity = (ticks_.target == MAJOR || ticks_.target == BOTH)?1:0;
  } else {
    float min_start = (ticks_.initial == MINOR || ticks_.initial == BOTH)?1:0;
    float min_end = (ticks_.target == MINOR || ticks_.target == BOTH)?1:0;
    float maj_start = (ticks_.initial == MAJOR || ticks_.initial == BOTH)?1:0;
    float maj_end = (ticks_.target == MAJOR || ticks_.target == BOTH)?1:0;

    min_tick_opacity = (1-ticks_.progress)*min_start + ticks_.progress*min_end;
    maj_tick_opacity = (1-ticks_.progress)*maj_start + ticks_.progress*maj_end;
  }
  min_tick_opacity *= tick_visibility_;
  maj_tick_opacity *= tick_visibility_;

  if (min_tick_opacity >= eps) {
    std::vector<float> min_ticks_pos_x;
    std::vector<float> min_ticks_pos_y;

    min_ticks_pos_x = calculateTicks_(x_axis_graph, ticks_x_min_);
    min_ticks_pos_y = calculateTicks_(y_axis_graph, ticks_y_min_);

    float min_sz = cosmetics_.min_tick_size*min_tick_opacity;
    setGlColor(cosmetics_.color*axis_opacity);
    drawTicks(x_axis, min_ticks_pos_x, min_sz, two_sidedness*min_sz, points);
    drawTicks(y_axis, min_ticks_pos_y, two_sidedness*min_sz, min_sz, points);
    if (boxiness >= 0.7) {
      vbo_ -> draw(points, GL_LINES);
      points.clear();

      float other_axes_opac = (boxiness - 0.7) / 0.3;
      setGlColor(cosmetics_.color*(axis_opacity*other_axes_opac));

      drawTicks(Rectangle(GlVertex2(axes_box_.start.x, axes_box_.end.y),
        axes_box_.end), min_ticks_pos_x, two_sidedness*min_sz, min_sz, points);
      drawTicks(Rectangle(GlVertex2(axes_box_.end.x, axes_box_.start.y),
        axes_box_.end), min_ticks_pos_y, min_sz, two_sidedness*min_sz, points);

      vbo_ -> draw(points, GL_LINES);
      points.clear();
    }
  }
  if (maj_tick_opacity >= eps) {
    if (maj_ticks_pos_x.empty())
      maj_ticks_pos_x = calculateTicks_(x_axis_graph, ticks_x_maj_);
    if (maj_ticks_pos_y.empty())
      maj_ticks_pos_y = calculateTicks_(y_axis_graph, ticks_y_maj_);

    float maj_sz = cosmetics_.maj_tick_size*maj_tick_opacity;
    setGlColor(cosmetics_.color*axis_opacity);
    drawTicks(x_axis, maj_ticks_pos_x, maj_sz, two_sidedness*maj_sz, points);
    drawTicks(y_axis, maj_ticks_pos_y, two_sidedness*maj_sz, maj_sz, points);
    if (boxiness >= 0.7) {
      vbo_ -> draw(points, GL_LINES);
      points.clear();

      float other_axes_opac = (boxiness - 0.7) / 0.3;
      setGlColor(cosmetics_.color*(axis_opacity*other_axes_opac));

      drawTicks(Rectangle(GlVertex2(axes_box_.start.x, axes_box_.end.y),
        axes_box_.end), maj_ticks_pos_x, two_sidedness*maj_sz, maj_sz, points);
      drawTicks(Rectangle(GlVertex2(axes_box_.end.x, axes_box_.start.y),
        axes_box_.end), maj_ticks_pos_y, maj_sz, two_sidedness*maj_sz, points);
    }
  }

  vbo_ -> draw(points, GL_LINES);
  points.clear();

  float box_opacity = box_*(1 - boxiness);
  if (box_opacity >= eps) {
    setGlColor(cosmetics_.box_color*box_opacity);
    drawBox(axes_box_, vbo_.get());
  }
}

static inline float convert(float x, float b0, float b1, float r0, float r1,
    Axes::ScalingType scaling)
{
  // XXX many of these values could be cached, at least while no animation
  // is taking place...
  switch (scaling) {
    case Axes::LINEAR:
      return b0 + (x - r0)*(b1 - b0)/(r1 - r0);
    case Axes::LOG:
      if ((x > 0 && r0 > 0 && r1 > 0) || (x < 0 && r0 < 0 && r1 < 0))
        return b0 + std::log(x/r0)*(b1 - b0)/std::log(r1/r0);
      else
        return 0;
    default:
      return 0;
  }
}

static inline float convert(float x, float b0, float b1, float r0, float r1,
    const DiscreteAnimated<Axes::ScalingType>& scaling)
{
  if (1 - scaling.progress < eps) { // no animation
    return convert(x, b0, b1, r0, r1, scaling.target);
  } else {
    float X1 = convert(x, b0, b1, r0, r1, scaling.initial);
    float X2 = convert(x, b0, b1, r0, r1, scaling.target);

    return (1 - scaling.progress)*X1 + scaling.progress*X2;
  }
}

GlVertex2 Axes::graphToScreen(const GlVertex2& p) const
{
  return GlVertex2(
    convert(p.x, axes_box_.start.x, axes_box_.end.x,
        range_.start.x, range_.end.x, scaling_x_),
    convert(p.y, axes_box_.start.y, axes_box_.end.y,
        range_.start.y, range_.end.y, scaling_y_)
  );
}

static inline float unconvert(float y, float b0, float b1, float r0, float r1,
    Axes::ScalingType scaling)
{
  // XXX many of these values could be cached, at least while no animation
  // is taking place...
  switch (scaling) {
    case Axes::LINEAR:
      return r0 + (r1 - r0)*(y - b0)/(b1 - b0);
    case Axes::LOG:
      if ((r0 > 0 && r1 > 0) || (r0 < 0 && r1 < 0))
        return r0*std::exp(std::log(r1/r0)*(y - b0)/(b1 - b0));
      else
        return 0;
    default:
      return 0;
  }
}

static inline float unconvert(float y, float b0, float b1, float r0, float r1,
    const DiscreteAnimated<Axes::ScalingType>& scaling)
{
  if (1 - scaling.progress < eps) { // no animation
    return unconvert(y, b0, b1, r0, r1, scaling.target);
  } else {
    float guess1 = unconvert(y, b0, b1, r0, r1, scaling.initial);
    float guess2 = unconvert(y, b0, b1, r0, r1, scaling.target);

    if (guess1 > guess2)
      std::swap(guess1, guess2);

    // use the secant method to find the solution
    const float eps = 1e-6;
    const size_t max_count = 48;
    size_t i = 0;

    // it's always true that f1 and f2 have opposite signs if the progress is
    // between 0 and 1
    float f1 = convert(guess1, b0, b1, r0, r1, scaling) - y;
    float f2 = convert(guess2, b0, b1, r0, r1, scaling) - y;

    while (i < max_count && std::abs(guess2 - guess1) > eps) {
      float guess_new = guess1 - f1*(guess2 - guess1)/(f2 - f1);
      if (guess_new == guess1 || guess_new == guess2) // reached machine eps
        break;
      float f_new = convert(guess_new, b0, b1, r0, r1, scaling) - y;
      if (std::abs(f_new) < eps)
        break;
      if ((f_new <= 0 && f1 >= 0) || (f_new >= 0 && f1 <= 0)) {
        guess2 = guess_new;
        f2 = f_new;
      } else {
        guess1 = guess_new;
        f1 = f_new;
      }
      ++i;
    }

    float guess = (f2 != f1)?(guess1 - f1*(guess2 - guess1)/(f2 - f1)):guess1;
    return guess;
  }
}

GlVertex2 Axes::screenToGraph(const GlVertex2& p) const
{
  return GlVertex2(
    unconvert(p.x, axes_box_.start.x, axes_box_.end.x,
        range_.start.x, range_.end.x, scaling_x_),
    unconvert(p.y, axes_box_.start.y, axes_box_.end.y,
        range_.start.y, range_.end.y, scaling_y_)
  );
}

float Axes::getTickSpacingLinearX(TicksType which, bool inst) const
{
  const TicksInfo& info = (which == MAJOR || which == BOTH)?
    ticks_x_maj_:ticks_x_min_;
  return animator_.get(&info.spacing_linear, inst);
}

float Axes::getTickSpacingLinearY(TicksType which, bool inst) const
{
  const TicksInfo& info = (which == MAJOR || which == BOTH)?
    ticks_y_maj_:ticks_y_min_;
  return animator_.get(&info.spacing_linear, inst);
}

float Axes::getTickSpacingLogX(TicksType which, bool inst) const
{
  const TicksInfo& info = (which == MAJOR || which == BOTH)?
    ticks_x_maj_:ticks_x_min_;
  return animator_.get(&info.spacing_log, inst);
}

float Axes::getTickSpacingLogY(TicksType which, bool inst) const
{
  const TicksInfo& info = (which == MAJOR || which == BOTH)?
    ticks_y_maj_:ticks_y_min_;
  return animator_.get(&info.spacing_log, inst);
}

void Axes::updateProperties()
{
  if (!properties_) // nothing to do
    return;

  if (properties_ -> count("type") > 0) {
    std::string type_str;
    Type type = getType();
    if (type == BOX)
      type_str = "box";
    else if (type == CROSS)
      type_str = "cross";
    else
      throw Exception("Axes::updateProperties: unhandled axis type!");

    properties_ -> put("type", type_str);
  }

  if (properties_ -> count("visible") > 0)
    properties_ -> put("visible", isVisible());

  if (properties_ -> count("ticks") > 0) {
    std::string type_str;
    TicksType type = getTickType();
    if (type == MINOR)
      type_str = "minor";
    else if (type == MAJOR)
      type_str = "major";
    else if (type == BOTH)
      type_str = "both";
    else
      throw Exception("Axes::updateProperties: unhandled ticks type!");

    properties_ -> put("ticks", type_str);
  }

  if (properties_ -> count("ticks_visible") > 0)
    properties_ -> put("ticks_visible", getTickVisibility());

  if (properties_ -> count("ticks_twosided") > 0)
    properties_ -> put("ticks_twosided", areTicksTwoSided());

  if (properties_ -> count("grid") > 0)
    properties_ -> put("grid", isGridVisible());

  if (properties_ -> count("bbox") > 0)
    properties_ -> put("bbox", isBoxVisible());

  if (properties_ -> count("clip") > 0)
    properties_ -> put("clip", getClipping());

  if (properties_ -> count("crosspoint") > 0)
    properties_ -> put("crosspoint", getCrossing());

  if (properties_ -> count("color") > 0)
    properties_ -> put("color", getColor());

  if (properties_ -> count("grid_color") > 0)
    properties_ -> put("grid_color", getGridColor());

  if (properties_ -> count("bbox_color") > 0)
    properties_ -> put("bbox_color", getBoxColor());

  if (properties_ -> count("mintick_size") > 0)
    properties_ -> put("mintick_size", getMinorTickSize());

  if (properties_ -> count("majtick_size") > 0)
    properties_ -> put("majtick_size", getMajorTickSize());

  updateAxisProperties_(X_AXIS, properties_ -> get_child("x"));
  updateAxisProperties_(Y_AXIS, properties_ -> get_child("y"));
}

void Axes::setProperties(Properties* props)
{
  properties_ = props;

  if (properties_ -> count("type") > 0) {
    std::string type = properties_ -> get<std::string>("type");
    if (type == "box")
      setType(BOX, "none");
    else if (type == "cross")
      setType(CROSS, "none");
    else
      throw Exception("Unknown axis type '"+type+"' (Axes::setProperties).");
  }

  if (properties_ -> count("visible") > 0)
    visibility_ = properties_ -> get<bool>("visible")?1:0;

  if (properties_ -> count("ticks") > 0) {
    std::string type = properties_ -> get<std::string>("ticks");
    if (type == "minor")
      setTickType(MINOR, "none");
    else if (type == "major")
      setTickType(MAJOR, "none");
    else if (type == "both")
      setTickType(BOTH, "none");
    else
      throw Exception("Unknown ticks type '"+type+"' (Axes::setProperties).");
  }

  if (properties_ -> count("ticks_visible") > 0)
    tick_visibility_ = properties_ -> get<bool>("ticks_visible")?1:0;

  if (properties_ -> count("ticks_twosided") > 0)
    ticks_twosided_ = properties_ -> get<bool>("ticks_twosided")?1:0;

  if (properties_ -> count("grid") > 0)
    grid_ = properties_ -> get<bool>("grid")?1:0;

  if (properties_ -> count("bbox") > 0)
    box_ = properties_ -> get<bool>("bbox")?1:0;

  if (properties_ -> count("clip") > 0)
    clip_ = properties_ -> get<bool>("clip");

  if (properties_ -> count("crosspoint") > 0)
    crossing_ = properties_ -> get<GlVertex2>("crosspoint");

  if (properties_ -> count("color") > 0)
    cosmetics_.color = properties_ -> get<GlColor4>("color");

  if (properties_ -> count("grid_color") > 0)
    cosmetics_.grid_color = properties_ -> get<GlColor4>("grid_color");

  if (properties_ -> count("bbox_color") > 0)
    cosmetics_.box_color = properties_ -> get<GlColor4>("bbox_color");

  if (properties_ -> count("mintick_size") > 0)
    cosmetics_.min_tick_size = properties_ -> get<float>("mintick_size");

  if (properties_ -> count("majtick_size") > 0)
    cosmetics_.maj_tick_size = properties_ -> get<float>("majtick_size");

  setAxisProperties_(X_AXIS, properties_ -> get_child("x"));
  setAxisProperties_(Y_AXIS, properties_ -> get_child("y"));
}

void Axes::setType(Type type, const std::string& trans)
{
  // XXX this needs to be made better
  type_.initial = type_.target;
  type_.target = type;
  animator_.doTransition(&type_.progress, 0, 1,
    getTransition_("type_change", trans));
}

void Axes::setVisibility(bool vis, const std::string& trans)
{
  animator_.redoTransition(&visibility_, vis?1:0,
    getTransition_("fade", trans));
}

void Axes::setTickType(TicksType type, const std::string& trans)
{
  // XXX this needs to be made better
  ticks_.initial = ticks_.target;
  ticks_.target = type;
  animator_.doTransition(&ticks_.progress, 0, 1,
    getTransition_("tick", trans));
}

void Axes::setTickVisibility(bool vis, const std::string& trans)
{
  animator_.redoTransition(&tick_visibility_, vis?1:0,
    getTransition_("fade", trans));
}

void Axes::setTicksTwoSided(bool two_sided, const std::string& trans)
{
  animator_.redoTransition(&ticks_twosided_, two_sided?1:0,
    getTransition_("tick", trans));
}

void Axes::setGridVisibility(bool vis, const std::string& trans)
{
  animator_.redoTransition(&grid_, vis?1:0, getTransition_("fade", trans));
}

void Axes::setBoxVisibility(bool vis, const std::string& trans)
{
  animator_.redoTransition(&box_, vis?1:0, getTransition_("fade", trans));
}

void Axes::setClippingArea(const Rectangle& r, const std::string& trans)
{
  animator_.redoTransition(&clipping_box_, r, getTransition_("zoom", trans));
}

void Axes::setExtents(const Rectangle& r, const std::string& trans)
{
  animator_.redoTransition(&axes_box_, r, getTransition_("zoom", trans));
}

void Axes::setRange(const Rectangle& r, const std::string& trans)
{
  animator_.redoTransition(&range_, r, getTransition_("zoom", trans));
}

void Axes::setCrossing(const GlVertex2& x, const std::string& trans)
{
  animator_.redoTransition(&crossing_, x, getTransition_("shift", trans));
}

void Axes::setColor(const GlColor4& color, const std::string& trans)
{
  animator_.redoTransition(&cosmetics_.color, color,
    getTransition_("fade", trans));
}

void Axes::setMinorTickSize(float sz, const std::string& trans)
{
  animator_.redoTransition(&cosmetics_.min_tick_size, sz,
    getTransition_("fade", trans));
}

void Axes::setMajorTickSize(float sz, const std::string& trans)
{
  animator_.redoTransition(&cosmetics_.maj_tick_size, sz,
    getTransition_("fade", trans));
}

void Axes::setGridColor(const GlColor4& color, const std::string& trans)
{
  animator_.redoTransition(&cosmetics_.grid_color, color,
    getTransition_("fade", trans));
}

void Axes::setBoxColor(const GlColor4& color, const std::string& trans)
{
  animator_.redoTransition(&cosmetics_.box_color, color,
    getTransition_("fade", trans));
}

void Axes::setScalingX(ScalingType type, const std::string& trans)
{
  // XXX this needs to be done better
  scaling_x_.initial = scaling_x_.target;
  scaling_x_.target = type;
  animator_.doTransition(&scaling_x_.progress, 0, 1,
    getTransition_("type_change", trans));
}

void Axes::setScalingY(ScalingType type, const std::string& trans)
{
  // XXX this needs to be done better
  scaling_y_.initial = scaling_y_.target;
  scaling_y_.target = type;
  animator_.doTransition(&scaling_y_.progress, 0, 1,
    getTransition_("type_change", trans));
}

void Axes::setTickOriginLinearX(float orig, const std::string& trans)
{
  animator_.redoTransition(&ticks_x_min_.origin_linear, orig,
    getTransition_("shift", trans));
}

void Axes::setTickOriginLinearY(float orig, const std::string& trans)
{
  animator_.redoTransition(&ticks_y_min_.origin_linear, orig,
    getTransition_("shift", trans));
}

void Axes::setTickOriginLogX(float orig, const std::string& trans)
{
  animator_.redoTransition(&ticks_x_min_.origin_log, orig,
    getTransition_("shift", trans));
}

void Axes::setTickOriginLogY(float orig, const std::string& trans)
{
  animator_.redoTransition(&ticks_y_min_.origin_log, orig,
    getTransition_("shift", trans));
}

void Axes::setTickSpacingLinearX(TicksType which, float sp,
    const std::string& trans)
{
  TicksInfo& info = (which == MAJOR || which == BOTH)?ticks_x_maj_:ticks_x_min_;
  animator_.redoTransition(&info.spacing_linear, sp,
    getTransition_("zoom", trans));
}

void Axes::setTickSpacingLinearY(TicksType which, float sp,
    const std::string& trans)
{
  TicksInfo& info = (which == MAJOR || which == BOTH)?ticks_y_maj_:ticks_y_min_;
  animator_.redoTransition(&info.spacing_linear, sp,
    getTransition_("zoom", trans));
}

void Axes::setTickSpacingLogX(TicksType which, float sp,
    const std::string& trans)
{
  TicksInfo& info = (which == MAJOR || which == BOTH)?ticks_x_maj_:ticks_x_min_;
  animator_.redoTransition(&info.spacing_log, sp,
    getTransition_("zoom", trans));
}

void Axes::setTickSpacingLogY(TicksType which, float sp,
    const std::string& trans)
{
  TicksInfo& info = (which == MAJOR || which == BOTH)?ticks_y_maj_:ticks_y_min_;
  animator_.redoTransition(&info.spacing_log, sp,
    getTransition_("zoom", trans));
}

void Axes::setTickSpacingX(ScalingType spacing, const std::string& trans)
{
  // XXX this can be done better
  ticks_x_min_.spacing_type.initial = ticks_x_min_.spacing_type.target;
  ticks_x_min_.spacing_type.target = spacing;
  animator_.doTransition(&ticks_x_min_.spacing_type.progress, 0, 1,
    getTransition_("type_change", trans));
}

void Axes::setTickSpacingY(ScalingType spacing, const std::string& trans)
{
  // XXX this can be done better
  ticks_y_min_.spacing_type.initial = ticks_y_min_.spacing_type.target;
  ticks_y_min_.spacing_type.target = spacing;
  animator_.doTransition(&ticks_y_min_.spacing_type.progress, 0, 1,
    getTransition_("type_change", trans));
}

static std::pair<float, BaseEasingPtr> trivial_trans(0, BaseEasingPtr());

const std::pair<float, BaseEasingPtr>&
    Axes::getTransition_(const std::string& name, const std::string& trans)
{
  if (trans == "none" || !transitions_) {
    return trivial_trans;
  } else if (trans.empty()) {
    return transitions_ -> get(name);
  } else {
    return transitions_ -> get(trans);
  }
}

void Axes::setAxisProperties_(AxisId which, const Properties& props)
{
  bool have_clip = (props.count("clip_range") > 0);
  bool have_screen = (props.count("screen_range") > 0);
  bool have_range = (props.count("range") > 0);
  bool have_scaling = (props.count("scaling") > 0);
  bool have_torigin_lin = (props.count("ticks_origin_linear") > 0);
  bool have_torigin_log = (props.count("ticks_origin_log") > 0);
  bool have_tspacing_type = (props.count("ticks_spacing") > 0);
  bool have_min_lin = (props.count("mintick_interval") > 0);
  bool have_maj_lin = (props.count("majtick_interval") > 0);
  bool have_min_log = (props.count("mintick_ratio") > 0);
  bool have_maj_log = (props.count("majtick_ratio") > 0);

  GlVertex2 clip_range, screen_range, range;
  ScalingType scaling;
  ScalingType tspacing_type;
  float t_origin_lin;
  float t_origin_log;
  float min_lin, maj_lin, min_log, maj_log;

  if (have_clip) clip_range = props.get<GlVertex2>("clip_range");
  if (have_screen) screen_range = props.get<GlVertex2>("screen_range");
  if (have_range) range = props.get<GlVertex2>("range");
  if (have_scaling) {
    std::string scaling_str = props.get<std::string>("scaling");
    if (scaling_str == "log")
      scaling = LOG;
    else if (scaling_str == "linear")
      scaling = LINEAR;
    else
      throw Exception("Unknown axis scaling type '" + scaling_str + "' "
        "(Axes::setAxisProperties).");
  }
  if (have_torigin_lin) t_origin_lin = props.get<float>("ticks_origin_linear");
  if (have_torigin_log) t_origin_log = props.get<float>("ticks_origin_log");
  if (have_tspacing_type) {
    std::string spacing_str = props.get<std::string>("ticks_spacing");
    if (spacing_str == "log")
      tspacing_type = LOG;
    else if (spacing_str == "linear")
      tspacing_type = LINEAR;
    else
      throw Exception("Unknown tick spacing type '" + spacing_str + "' "
        "(Axes::setAxisProperties).");
  }
  if (have_min_lin) min_lin = props.get<float>("mintick_interval");
  if (have_maj_lin) maj_lin = props.get<float>("majtick_interval");
  if (have_min_log) min_log = props.get<float>("mintick_ratio");
  if (have_maj_log) maj_log = props.get<float>("majtick_ratio");
 
  if (which == X_AXIS) {
    if (have_clip) {
      clipping_box_.start.x = clip_range.x;
      clipping_box_.end.x = clip_range.y;
    }

    if (have_screen) {
      axes_box_.start.x = screen_range.x;
      axes_box_.end.x = screen_range.y;
    }

    if (have_range) {
      range_.start.x = range.x;
      range_.end.x = range.y;
    }

    if (have_scaling)
      scaling_x_ = scaling;

    if (have_torigin_lin) {
      ticks_x_min_.origin_linear = t_origin_lin;
      ticks_x_maj_.origin_linear = t_origin_lin;
    }
    if (have_torigin_log) {
      ticks_x_min_.origin_log = t_origin_log;
      ticks_x_maj_.origin_log = t_origin_log;
    }

    if (have_tspacing_type) {
      ticks_x_min_.spacing_type = tspacing_type;
      ticks_x_maj_.spacing_type = tspacing_type;
    }

    if (have_min_lin) ticks_x_min_.spacing_linear = min_lin;
    if (have_maj_lin) ticks_x_maj_.spacing_linear = maj_lin;
    if (have_min_log) ticks_x_min_.spacing_log = min_log;
    if (have_maj_log) ticks_x_maj_.spacing_log = maj_log;
  } else {
    if (have_clip) {
      clipping_box_.start.y = clip_range.x;
      clipping_box_.end.y = clip_range.y;
    }

    if (have_screen) {
      axes_box_.start.y = screen_range.x;
      axes_box_.end.y = screen_range.y;
    }

    if (have_range) {
      range_.start.y = range.x;
      range_.end.y = range.y;
    }

    if (have_scaling)
      scaling_y_ = scaling;

    if (have_torigin_lin) {
      ticks_y_min_.origin_linear = t_origin_lin;
      ticks_y_maj_.origin_linear = t_origin_lin;
    }
    if (have_torigin_log) {
      ticks_y_min_.origin_log = t_origin_log;
      ticks_y_maj_.origin_log = t_origin_log;
    }

    if (have_tspacing_type) {
      ticks_y_min_.spacing_type = tspacing_type;
      ticks_y_maj_.spacing_type = tspacing_type;
    }

    if (have_min_lin) ticks_y_min_.spacing_linear = min_lin;
    if (have_maj_lin) ticks_y_maj_.spacing_linear = maj_lin;
    if (have_min_log) ticks_y_min_.spacing_log = min_log;
    if (have_maj_log) ticks_y_maj_.spacing_log = maj_log;
  }
}

void Axes::updateAxisProperties_(AxisId which, Properties& props)
{
  bool have_clip = (props.count("clip_range") > 0);
  bool have_screen = (props.count("screen_range") > 0);
  bool have_range = (props.count("range") > 0);
  bool have_scaling = (props.count("scaling") > 0);
  bool have_torigin_lin = (props.count("ticks_origin_linear") > 0);
  bool have_torigin_log = (props.count("ticks_origin_log") > 0);
  bool have_tspacing_type = (props.count("ticks_spacing") > 0);
  bool have_min_lin = (props.count("mintick_interval") > 0);
  bool have_maj_lin = (props.count("majtick_interval") > 0);
  bool have_min_log = (props.count("mintick_ratio") > 0);
  bool have_maj_log = (props.count("majtick_ratio") > 0);

  if (which == X_AXIS) {
    if (have_clip) {
      const Rectangle& clip = getClippingArea();
      props.put("clip_range", GlVertex2(clip.start.x, clip.end.x));
    }
    if (have_screen) {
      const Rectangle& screen = getExtents();
      props.put("screen_range", GlVertex2(screen.start.x, screen.end.x));
    }
    if (have_range) {
      const Rectangle& range = getRange();
      props.put("range", GlVertex2(range.start.x, range.end.x));
    }
    if (have_scaling) {
      std::string str;
      ScalingType scaling = getScalingX();
      if (scaling == LOG)
        str = "log";
      else if (scaling == LINEAR)
        str = "linear";
      else
        throw Exception("axes::updateAxisProperties: unhandled scaling type!");

      props.put("scaling", str);
    }

    if (have_torigin_lin) {
      props.put("ticks_origin_linear", getTickOriginLinearX());
    }
    if (have_torigin_log) {
      props.put("ticks_origin_log", getTickOriginLogX());
    }

    if (have_tspacing_type) {
      std::string str;
      ScalingType spacing = getTickSpacingX();
      if (spacing == LOG)
        str = "log";
      else if (spacing == LINEAR)
        str = "linear";
      else
        throw Exception("axes::updateAxisProperties: unhandled tick spacing "
          "type!");

      props.put("ticks_spacing", str);
    }

    if (have_min_lin)
      props.put("mintick_interval", getTickSpacingLinearX(MINOR));
    if (have_maj_lin)
      props.put("majtick_interval", getTickSpacingLinearX(MAJOR));
    if (have_min_log)
      props.put("mintick_ratio", getTickSpacingLogX(MINOR));
    if (have_maj_log)
      props.put("majtick_ratio", getTickSpacingLogX(MAJOR));
  } else {
    if (have_clip) {
      const Rectangle& clip = getClippingArea();
      props.put("clip_range", GlVertex2(clip.start.y, clip.end.y));
    }
    if (have_screen) {
      const Rectangle& screen = getExtents();
      props.put("screen_range", GlVertex2(screen.start.y, screen.end.y));
    }
    if (have_range) {
      const Rectangle& range = getRange();
      props.put("range", GlVertex2(range.start.y, range.end.y));
    }
    if (have_scaling) {
      std::string str;
      ScalingType scaling = getScalingY();
      if (scaling == LOG)
        str = "log";
      else if (scaling == LINEAR)
        str = "linear";
      else
        throw Exception("axes::updateAxisProperties: unhandled scaling type!");

      props.put("scaling", str);
    }

    if (have_torigin_lin) {
      props.put("ticks_origin_linear", getTickOriginLinearY());
    }
    if (have_torigin_log) {
      props.put("ticks_origin_log", getTickOriginLogY());
    }

    if (have_tspacing_type) {
      std::string str;
      ScalingType spacing = getTickSpacingY();
      if (spacing == LOG)
        str = "log";
      else if (spacing == LINEAR)
        str = "linear";
      else
        throw Exception("axes::updateAxisProperties: unhandled tick spacing "
          "type!");

      props.put("ticks_spacing", str);
    }

    if (have_min_lin)
      props.put("mintick_interval", getTickSpacingLinearY(MINOR));
    if (have_maj_lin)
      props.put("majtick_interval", getTickSpacingLinearY(MAJOR));
    if (have_min_log)
      props.put("mintick_ratio", getTickSpacingLogY(MINOR));
    if (have_maj_log)
      props.put("majtick_ratio", getTickSpacingLogY(MAJOR));
  }
}
