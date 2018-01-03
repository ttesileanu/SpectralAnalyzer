/** @file vbo_info.h
 *  @brief Defines a structures that is useful for managing VBOs.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_VBO_INFO_H_
#define GLUTILS_VBO_INFO_H_

#include "glutils/gl_incs.h"

/** @brief This structure helps with @a Vbo::draw.
 *
 *  The @a draw method of the @a Vbo class expects the vector of elements that
 *  are to be drawn to indicate the kind of data that is stored -- how many
 *  space coordinates and of what type, how many texture coordinates and of
 *  what type, etc. These are assumed to be structures that have public members
 *  @a n, @a type, and @a size. The @a VboInfo structure provides these
 *  members, and automatically fills out the @a size field for typical data
 *  types.
 *
 *  @see Vbo::draw
 */
struct VboInfo {
  /** @brief Create a @a VboInfo structure, containing @a N elements of type
   *         @a t. The optional argument @a s gives the size of the data in
   *         bytes; this can be used to override the default size calculation.
   */
  VboInfo(GLint N, GLenum t, GLsizei s = 0) {
    n = N;
    type = t;
    if (s == 0) {
      switch (t) {
        case GL_FLOAT:
          size = N*sizeof(GLfloat);
          break;
        case GL_DOUBLE:
          size = N*sizeof(GLdouble);
          break;
        case GL_INT:
          size = N*sizeof(GLint);
          break;
        default:
          size = 0;
      }
    } else {
      size = N*s;
    }
  }

  /// Number of elements
  GLint n;
  /// Type of elements (see the OpenGL documentation for possible values)
  GLenum type;
  /// Size in bytes of @a n elements of type @a type.
  GLsizei size;
};

#endif
