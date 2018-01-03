/** @file texture.h
 *  @brief Defines an RAII wrapper class for OpenGL textures.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_TEXTURE_H_
#define GLUTILS_TEXTURE_H_

#include <boost/noncopyable.hpp>

#include "glutils/gl_incs.h"

class Texture : boost::noncopyable {
 public:
  /// Make a new texture.
  Texture() { generate_(); }

  // XXX this probably needs a lot more options..
  /// Make a new texture of the given size. This binds the texture.
  Texture(unsigned width, unsigned height);

  /// Destroy the texture.
  ~Texture() { glDeleteTextures(1, &label_); }

  /// Bind the texture.
  void bind() { glBindTexture(GL_TEXTURE_2D, label_); }

  /// Unbind any texture.
  static void unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

  /// Get the integer label for this texture.
  GLuint getLabel() const { return label_; }

 protected:
  void generate_() { glGenTextures(1, &label_); }

  GLuint      label_;
};

#endif
