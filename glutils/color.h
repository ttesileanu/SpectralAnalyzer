/** @file color.h
 *  @brief Defines structures to aid in using OpenGL colors.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_COLOR_H_
#define GLUTILS_COLOR_H_

#include <iostream>
#include <stdexcept>

#include <cctype>

#include "glutils/vbo_info.h"
#include "glutils/gl_incs.h"

struct GlColor4 {
  /// Red.
  GLfloat r;
  /// Green.
  GLfloat g;
  /// Blue.
  GLfloat b;
  /// Alpha (opacity).
  GLfloat a;

  /// Empty constructor.
  GlColor4() {}
  /// Constructor with initialization.
  GlColor4(GLfloat rr, GLfloat gg, GLfloat bb, GLfloat aa = 1) : r(rr), g(gg),
    b(bb), a(aa) {}
};

/// A vertex.
struct GlColoredVertex2 {
  /// X coordinate.
  GLfloat   x;
  /// Y coordinate.
  GLfloat   y;
  /// Color
  GlColor4  color;

  static const VboInfo vertexInfo;
  static const VboInfo textureInfo;
  static const VboInfo colorInfo;

  /// Empty constructor.
  GlColoredVertex2() {}
  /// Constructor with initialization.
  GlColoredVertex2(GLfloat a, GLfloat b, const GlColor4& c) : x(a), y(b),
      color(c) {}
};

/// Set the color in OpenGL.
inline void setGlColor(const GlColor4& col)
{
  glColor4f(col.r, col.g, col.b, col.a);
}

/// Read @a GlColor4 from stream.
std::istream& operator>>(std::istream& in, GlColor4& c);

/// Write @a GlColor4 to stream.
inline std::ostream& operator<<(std::ostream& out, const GlColor4& c)
{
  return out << c.r << "," << c.g << "," << c.b << "," << c.a;
}

/// Addition of colors.
inline GlColor4 operator+(const GlColor4& a, const GlColor4& b)
{
  return GlColor4(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
}

/// Multiplication of a color by a scalar.
inline GlColor4 operator*(const GlColor4& col, float a)
{
  return GlColor4(col.r*a, col.g*a, col.b*a, col.a*a);
}

/// Multiplication of a color by a scalar.
inline GlColor4 operator*(float a, const GlColor4& col)
{
  return GlColor4(col.r*a, col.g*a, col.b*a, col.a*a);
}

#endif
