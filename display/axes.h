/** @file axes.h
 *  @brief Defines classes for managing and drawing a system of axes.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef AXES_H_
#define AXES_H_

#include <boost/scoped_ptr.hpp>

#include "animation/animator.h"
#include "glutils/color.h"
#include "glutils/geometry.h"
#include "glutils/vbo.h"
#include "glutils/gl_incs.h"
#include "utils/forward_defs.h"
#include "utils/misc.h"
#include "utils/properties.h"

/// Class the manages and displays a system of axes.
class Axes {
 public:
  /// Axes type.
  enum Type {CROSS, BOX};

  /// Ticks type.
  enum TicksType {MAJOR, MINOR, BOTH};

  /// Scaling type.
  enum ScalingType {LINEAR, LOG};

  /// Empty constructor.
  Axes() : type_(CROSS), visibility_(1), ticks_(MAJOR), tick_visibility_(1),
    ticks_twosided_(1), grid_(1), box_(0), clip_(false),
    clipping_box_(0, 0, 1000, 1000), axes_box_(10, 10, 600, 400),
    range_(-1, -1, 1, 1), crossing_(0, 0), scaling_x_(LINEAR),
    scaling_y_(LINEAR), properties_(0) {}

  /// Draw the axes.
  void draw();

  /// Calculate the screen coordinates of a point in graph space.
  GlVertex2 graphToScreen(const GlVertex2& p) const;

  /// Calculate the graph coordinates of a point in screen space.
  GlVertex2 screenToGraph(const GlVertex2& p) const;

  /** @brief Decide whether a point is within the clipping range.
   *
   *  Always returns @a true if clipping is disabled.
   */
  bool isWithinClipping(const GlVertex2& p) const {
    return (!getClipping() || (p.x >= clipping_box_.start.x &&
      p.x <= clipping_box_.end.x && p.y >= clipping_box_.start.y &&
      p.y <= clipping_box_.end.y));
  }

  /** @brief Return a clipped version of the point.
   *
   *  Always returns the original point if clipping is disabled.
   */
  GlVertex2 getClipped(const GlVertex2& p) const {
    if (!getClipping()) return p;
    GlVertex2 res = p;
    if (res.x < clipping_box_.start.x) res.x = clipping_box_.start.x;
    if (res.x > clipping_box_.end.x) res.x = clipping_box_.end.x;
    if (res.y < clipping_box_.start.y) res.y = clipping_box_.start.y;
    if (res.y > clipping_box_.end.y) res.y = clipping_box_.end.y;

    return res;
  }

  /// Update the animations.
  void updateAnimations() { animator_.update(); }

  /// @name getters
  // @{

  /// Get the type of axes.
  Type getType() const { return type_.target; }

  /// Get the visibility of axes.
  bool isVisible() const { return animator_.getTarget(&visibility_) > 0.5; }

  /// Get the type of ticks.
  TicksType getTickType() const { return ticks_.target; }

  /// Get the visibility of ticks.
  bool getTickVisibility() const
    { return animator_.getTarget(&tick_visibility_) > 0.5; }

  /// Find out whether ticks are two-sided or one-sided.
  bool areTicksTwoSided() const
    { return animator_.getTarget(&ticks_twosided_) > 0.5; }

  /// Get the visibility of the grid.
  bool isGridVisible() const { return animator_.getTarget(&grid_) > 0.5; }

  /// Get the visibility of the bounding box.
  bool isBoxVisible() const { return animator_.getTarget(&box_) > 0.5; }

  /// Find out whether we're supposed to clip or not.
  bool getClipping() const { return clip_; }

  /// Get the extents of the clipping box.
  const Rectangle& getClippingArea(bool instantaneous = false) const
    { return animator_.get(&clipping_box_, instantaneous); }

  /// Get the extents of the axes in screen space.
  const Rectangle& getExtents(bool instantaneous = false) const
    { return animator_.get(&axes_box_, instantaneous); }

  /// Get the extents of the axes in graph space.
  const Rectangle& getRange(bool instantaneous = false) const
    { return animator_.get(&range_, instantaneous); }

  /// Get the point where the axes cross (in graph space).
  const GlVertex2& getCrossing(bool instantaneous = false) const
    { return animator_.get(&crossing_, instantaneous); }

  /// Get the color of the axes.
  const GlColor4& getColor(bool instantaneous = false) const
    { return animator_.get(&cosmetics_.color, instantaneous); }

  /// Get the size of the minor ticks.
  float getMinorTickSize(bool instantaneous = false) const
    { return animator_.get(&cosmetics_.min_tick_size, instantaneous); }

  /// Get the size of the major ticks.
  float getMajorTickSize(bool instantaneous = false) const
    { return animator_.get(&cosmetics_.maj_tick_size, instantaneous); }

  /// Get the color of the grid.
  const GlColor4& getGridColor(bool instantaneous = false) const
    { return animator_.get(&cosmetics_.grid_color, instantaneous); }

  /// Get the color of the bounding box.
  const GlColor4& getBoxColor(bool instantaneous = false) const
    { return animator_.get(&cosmetics_.box_color, instantaneous); }

  /// Get the scaling type for the x axis.
  ScalingType getScalingX() const { return scaling_x_.target; }

  /// Get the scaling type for the y axis.
  ScalingType getScalingY() const { return scaling_y_.target; }

  /// Get the origin of x ticks, when using linear spacing.
  float getTickOriginLinearX(bool instantaneous = false) const
    { return animator_.get(&ticks_x_min_.origin_linear, instantaneous); }

  /// Get the origin of y ticks, when using linear spacing.
  float getTickOriginLinearY(bool instantaneous = false) const
    { return animator_.get(&ticks_y_min_.origin_linear, instantaneous); }

  /// Get the origin of x ticks, when using log spacing.
  float getTickOriginLogX(bool instantaneous = false) const
    { return animator_.get(&ticks_x_min_.origin_log, instantaneous); }

  /// Get the origin of y ticks, when using log spacing.
  float getTickOriginLogY(bool instantaneous = false) const
    { return animator_.get(&ticks_y_min_.origin_log, instantaneous); }

  /// Get the spacing interval used for x ticks, when using linear spacing.
  float getTickSpacingLinearX(TicksType which, bool inst = false) const;

  /// Get the spacing interval used for y ticks, when using linear spacing.
  float getTickSpacingLinearY(TicksType which, bool inst = false) const;

  /// Get the spacing ratio used for x ticks, when using log spacing.
  float getTickSpacingLogX(TicksType which, bool inst = false) const;

  /// Get the spacing ratio used for y ticks, when using log spacing.
  float getTickSpacingLogY(TicksType which, bool inst = false) const;

  /// Get spacing type for x ticks.
  ScalingType getTickSpacingX() const
    { return ticks_x_min_.spacing_type.target; }

  /// Get spacing type for y ticks.
  ScalingType getTickSpacingY() const
    { return ticks_y_min_.spacing_type.target; }

  // @}
  // (getters)

  /// Update the settings.
  void updateProperties();
  
  /// @name setters
  // @{
  /// Get access to settings. This also sets all the properties.
  void setProperties(Properties* props);

  /// Get access to the transition store.
  void setTransitionStore(const TransitionStorePtr& trans)
    { transitions_ = trans; }

  /// Change axis type.
  void setType(Type type, const std::string& trans = std::string());

  /// Change visibility of axes.
  void setVisibility(bool vis, const std::string& trans = std::string());

  /// Flip visibility of axes.
  void flipVisibility(const std::string& trans = std::string())
    { setVisibility(animator_.getTarget(&visibility_) < 0.5, trans); }

  /// Change type of ticks.
  void setTickType(TicksType type, const std::string& trans = std::string());

  /// Change tick visibility.
  void setTickVisibility(bool vis, const std::string& trans = std::string());

  /// Flip tick visibility.
  void flipTickVisibility(const std::string& trans = std::string())
    { setTickVisibility(animator_.getTarget(&tick_visibility_) < 0.5, trans); }

  /// Change tick two-sidedness.
  void setTicksTwoSided(bool two_sided,
    const std::string& trans = std::string());

  /// Change grid visibility.
  void setGridVisibility(bool vis, const std::string& trans = std::string());

  /// Flip visibility of grid.
  void flipGridVisibility(const std::string& trans = std::string())
    { setGridVisibility(animator_.getTarget(&grid_) < 0.5, trans); }

  /// Change visibility of bounding box.
  void setBoxVisibility(bool vis, const std::string& trans = std::string());

  /// Flip visibility of bounding box.
  void flipBoxVisibility(const std::string& trans = std::string())
    { setBoxVisibility(animator_.getTarget(&box_) < 0.5, trans); }

  /// Set clipping state.
  void setClipping(bool c) { clip_ = c; }

  /// Set the area used for clipping.
  void setClippingArea(const Rectangle& r,
    const std::string& trans = std::string());

  /// Set the extents of the axes in screen space.
  void setExtents(const Rectangle& r, const std::string& trans = std::string());

  /// Set the extents of the axes in graph space.
  void setRange(const Rectangle& r, const std::string& trans = std::string());

  /// Set the position where the axes cross for @a type_ == @a CROSS.
  void setCrossing(const GlVertex2& x,
    const std::string& trans = std::string());

  /// Change the color of the axes.
  void setColor(const GlColor4& color,
    const std::string& trans = std::string());

  /// Change the size of the minor ticks.
  void setMinorTickSize(float sz, const std::string& trans = std::string());

  /// Change the size of the major ticks.
  void setMajorTickSize(float sz, const std::string& trans = std::string());

  /// Change the color of the grid
  void setGridColor(const GlColor4& color,
    const std::string& trans = std::string());

  /// Change the color of the bounding box.
  void setBoxColor(const GlColor4& color,
    const std::string& trans = std::string());

  /// Set scaling type for x axis (@a LINEAR or @a LOG).
  void setScalingX(ScalingType type, const std::string& trans = std::string());

  /// Set scaling type for y axis (@a LINEAR or @a LOG).
  void setScalingY(ScalingType type, const std::string& trans = std::string());

  /// Set the origin for x ticks, when using linear spacing.
  void setTickOriginLinearX(float orig,
    const std::string& trans = std::string());

  /// Set the origin for y ticks, when using linear spacing.
  void setTickOriginLinearY(float orig,
    const std::string& trans = std::string());

  /// Set the origin for x ticks, when using log spacing.
  void setTickOriginLogX(float orig, const std::string& trans = std::string());

  /// Set the origin for y ticks, when using log spacing.
  void setTickOriginLogY(float orig, const std::string& trans = std::string());

  /// Set the spacing interval for x ticks, when using linear spacing.
  void setTickSpacingLinearX(TicksType which, float sp,
    const std::string& trans = std::string());

  /// Set the spacing interval for y ticks, when using linear spacing.
  void setTickSpacingLinearY(TicksType which, float sp,
    const std::string& trans = std::string());

  /// Set the spacing ratio for x ticks, when using log spacing.
  void setTickSpacingLogX(TicksType which, float sp,
    const std::string& trans = std::string());

  /// Set the spacing ratio for y ticks, when using log spacing.
  void setTickSpacingLogY(TicksType which, float sp,
    const std::string& trans = std::string());

  /// Set spacing type for x ticks.
  void setTickSpacingX(ScalingType spacing,
    const std::string& trans = std::string());

  /// Set spacing type for y ticks.
  void setTickSpacingY(ScalingType spacing,
    const std::string& trans = std::string());

  // @}
  // (setters)

 private:
  /// Structure holding cosmetic information about the axes.
  struct AxesCosmetics {
    /// Color of the axes.
    GlColor4      color;
    /// Size of the minor ticks.
    float         min_tick_size;
    /// Size of the major ticks.
    float         maj_tick_size;
    /// Color of the grid.
    GlColor4      grid_color;
    /// Color of the bounding box.
    GlColor4      box_color;

    /// Empty constructor.
    AxesCosmetics() : color(1, 0.3, 0), min_tick_size(3), maj_tick_size(5),
      grid_color(0.3, 0.3, 0.3), box_color(0.6, 0.6, 0.6) {}
  };

  /// Information about tick distribution.
  struct TicksInfo {
    /// Spacing type (linear or log) for ticks and grid.
    DiscreteAnimated<ScalingType> spacing_type;
    /// Spacing when using linear space.
    float                         spacing_linear;
    /// Spacing when using log space.
    float                         spacing_log;
    /// Origin for linear space.
    float                         origin_linear;
    /// Origin for log space.
    float                         origin_log;

    /// Empty constructor.
    TicksInfo() : spacing_type(LINEAR), spacing_linear(0.1),
      spacing_log(2), origin_linear(0), origin_log(1) {}
  };

  /// An identifier for the axes.
  enum AxisId { X_AXIS, Y_AXIS };

  /** @brief Calculate positions of ticks.
   *
   *  The axis coordinates are given in graph-space. The resulting values
   *  are in screen-coordinates, and they correspond to the non-constant
   *  component in @a axis_graph.
   */
  std::vector<float> calculateTicks_(const Rectangle& axis_graph,
    const TicksInfo& ticks_info);
  /** @brief Get the transition with the given name, or an abrupt transition if
   *         @a immediate = @a true or the transition store hasn't been set.
   */
  const std::pair<float, BaseEasingPtr>& getTransition_(const std::string& name,
      const std::string& trans);
  /// Set properties for the given axis.
  void setAxisProperties_(AxisId which, const Properties& props);
  /// Update properties for the given axis.
  void updateAxisProperties_(AxisId which, Properties& props);

  /// Type of axes.
  DiscreteAnimated<Type>        type_;
  /// Visibility of axes.
  float                         visibility_;
  /// Type of ticks.
  DiscreteAnimated<TicksType>   ticks_;
  /// Visibility of ticks.
  float                         tick_visibility_;
  /** @brief Whether the ticks are two-sided. Type @a float for animation.
   *
   *  Ticks are always one-sided when the @a type_ is @a BOX.
   */
  float                         ticks_twosided_;
  /** @brief Whether a grid is to be drawn. Type @a float so it's easy to
      animate.
   *
   *  The grid is only drawn if @a type_ != @a NO_AXES, or @a box == @a true.
   */
  float                         grid_;
  /** @brief Whether a bounding box is to be drawn. Type @a float so it's easy
   *  to animate.
   *
   *  This only makes sense if @a type_ == @a CROSS or @a NO_AXES, and extends
   *  as far as the axes do.
   */
  float                         box_;
  /// Whether to restrict drawing to the clipping box.
  bool                          clip_;
  /// The clipping box.
  Rectangle                     clipping_box_;
  /// The area where the axes are drawn.
  Rectangle                     axes_box_;
  /// The range in graph-space represented by the axes.
  Rectangle                     range_;
  /// Position (in graph-space) where the axes cross if @a type_ == @a CROSS.
  GlVertex2                     crossing_;
  /// Information about how to display the axes.
  AxesCosmetics                 cosmetics_;
  /// Scaling type (linear or log) for x axis.
  DiscreteAnimated<ScalingType> scaling_x_;
  /// Scaling type (linear or log) for y axis.
  DiscreteAnimated<ScalingType> scaling_y_;
  /// Minor tick properties for the x axis.
  TicksInfo                     ticks_x_min_;
  /// Minor tick properties for the y axis.
  TicksInfo                     ticks_y_min_;
  /// Major tick properties for the x axis.
  TicksInfo                     ticks_x_maj_;
  /// Major tick properties for the y axis.
  TicksInfo                     ticks_y_maj_;

  /// Animator object.
  Animator                      animator_;

  /// Properties of the axes system -- for reading/writing to ini file.
  Properties*                   properties_;

  /// Store for transitions.
  TransitionStorePtr            transitions_;

  /// The VBO in use by the object.
  boost::scoped_ptr<Vbo>        vbo_;
};

#endif
