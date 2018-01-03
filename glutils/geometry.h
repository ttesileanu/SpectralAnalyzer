/** @file geometry.h
 *  @brief Defines structures to aid in handling geometry and texture
           coordinates.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_GEOMETRY_H_
#define GLUTILS_GEOMETRY_H_

#include <iostream>
#include <stdexcept>

#include <cctype>
#include <cmath>

#include "glutils/vbo_info.h"
#include "glutils/gl_incs.h"

/// A vertex.
struct GlVertex2 {
  /// X coordinate.
  GLfloat x;
  /// Y coordinate.
  GLfloat y;

  static const VboInfo vertexInfo;
  static const VboInfo textureInfo;
  static const VboInfo colorInfo;

  /// Empty constructor.
  GlVertex2() {}
  /// Constructor with initialization.
  GlVertex2(GLfloat a, GLfloat b) : x(a), y(b) {}
};

/// A vertex with texture coordinates (useful in VBOs).
struct GlVertexTex2 {
  /// X coordinate.
  GLfloat x;
  /// Y coordinate.
  GLfloat y;

  /// S coordinate.
  GLfloat s;
  /// T coordinate.
  GLfloat t;

  static const VboInfo vertexInfo;
  static const VboInfo textureInfo;
  static const VboInfo colorInfo;

  /// Empty constructor.
  GlVertexTex2() {}
  /// Constructor with initialization.
  GlVertexTex2(GLfloat a, GLfloat b, GLfloat c, GLfloat d) : x(a), y(b),
    s(c), t(d) {}
  /// Constructor with initialization.
  GlVertexTex2(const GlVertex2& pos, const GlVertex2& tex) : x(pos.x), y(pos.y),
    s(tex.x), t(tex.y) {}
};

/// Read @a GlVertex2 from stream.
std::istream& operator>>(std::istream& in, GlVertex2& v);

/// Write @a GlVertex2 to stream.
inline std::ostream& operator<<(std::ostream& out, const GlVertex2& v)
{
  return out << v.x << "," << v.y;
}

/// Addition of vectors.
inline GlVertex2 operator+(const GlVertex2& a, const GlVertex2& b)
{
  return GlVertex2(a.x + b.x, a.y + b.y);
}

/// Subtraction of vectors.
inline GlVertex2 operator-(const GlVertex2& a, const GlVertex2& b)
{
  return GlVertex2(a.x - b.x, a.y - b.y);
}

/// Multiplication by scalar.
inline GlVertex2 operator*(const GlVertex2& a, float b)
{
  return GlVertex2(b*a.x, b*a.y);
}

/// Multiplication by scalar.
inline GlVertex2 operator*(float a, const GlVertex2& b)
{
  return GlVertex2(a*b.x, a*b.y);
}

/// Division by scalar.
inline GlVertex2 operator/(const GlVertex2& a, float b)
{
  return GlVertex2(a.x/b, a.y/b);
}

/// Unary minus.
inline GlVertex2 operator-(const GlVertex2& a)
{
  return GlVertex2(-a.x, -a.y);
}

/// Normalization to unit vector.
inline GlVertex2 normalize(const GlVertex2& a)
{
  float factor = 1.0 / std::sqrt(a.x*a.x + a.y*a.y);
  return factor*a;
}

/// A rectangle.
struct Rectangle {
  /// Bottom-left corner.
  GlVertex2   start;
  /// Top-right corner.
  GlVertex2   end;

  /// Empty constructor.
  Rectangle() {}
  /// Constructor with initialization.
  Rectangle(const GlVertex2& s, const GlVertex2& e) : start(s), end(e) {}
  /// Constructor with initialization.
  Rectangle(GLfloat a, GLfloat b, GLfloat c, GLfloat d) : start(a, b),
    end(c, d) {}
};

/// Addition of rectangles.
inline Rectangle operator+(const Rectangle& a, const Rectangle& b)
{
  return Rectangle(a.start + b.start, a.end + b.end);
}

/// Multiplication of rectangle by scalar.
inline Rectangle operator*(const Rectangle& a, float b)
{
  return Rectangle(b*a.start, b*a.end);
}

/// Multiplication of rectangle by scalar.
inline Rectangle operator*(float a, const Rectangle& b)
{
  return Rectangle(a*b.start, a*b.end);
}

#endif
