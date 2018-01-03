/** @file vbo.h
 *  @brief Defines an RAII wrapper class for vertex buffer objects (VBOs).
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_VBO_H_
#define GLUTILS_VBO_H_

#include <boost/noncopyable.hpp>

#include "glutils/gl_incs.h"
#include "utils/exception.h"

/** @brief An RAII wrapper for vertex buffer objects (VBOs).
 *
 *  You can enable auto-resizing, when the VBO grows automatically if more
 *  data needs to be sent to it. @see setAutoResize()
 */
class Vbo : boost::noncopyable {
 public:
  /// Create new VBO of given size (in bytes). This also binds the VBO.
  explicit Vbo(unsigned size);
  /// Destroy the VBO. This also unbinds any VBO.
  ~Vbo();

  /// Bind the VBO.
  void bind() { glBindBuffer(GL_ARRAY_BUFFER, label_); }
  /// Unbind any VBO.
  static void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  /// Send the data to the VBO. This binds the VBO.
  template <class Iterator>
  void update(Iterator begin, Iterator end, size_t offset = 0) {
    bind();
    size_t content_size = (end - begin)*sizeof(*begin);
    if (content_size + offset > size_) {
      if (auto_resize_) {
        resize(content_size);
      } else {
        throw Exception("VBO update too large for the buffer.");
      }
    }
    glBufferSubData(GL_ARRAY_BUFFER, offset, content_size, &(*begin));
  }

  /// Send the data to the VBO. This binds the VBO.
  template <class Container>
  void update(const Container& data, size_t offset = 0)
    { update(data.begin(), data.end(), offset); }

  /// Get the integer label for the VBO.
  GLuint getLabel() const { return label_; }

  /** @brief Update the VBO, and draw it in the given @a mode.
   *
   *  Use a non-zero @a offset to start writing at a different position in the
   *  buffer than the beginning.
   *
   *  The object pointed to by the iterator should have two member structs,
   *  @a vertexInfo, and @a textureInfo; each of these should have
   *  members @a n, @a size, and @a type. @a n is the number of components for
   *  either vertices or texture coordinates; @a size is the size of all the
   *  components, in bytes; and @a type is the OpenGL typedef for the data
   *  type of the components. See the @a VboInfo template for an easy way
   *  to create the @a vertexInfo and @a textureInfo members.
   */
  template <class Iterator>
  void draw(Iterator begin, Iterator end, GLenum mode, GLint offset = 0)
  {
    bind();
    update(begin, end, offset);

    GLsizei stride = begin -> vertexInfo.size +
      begin -> textureInfo.size + begin -> colorInfo.size;
    size_t vertexOffset = 0;
    size_t textureOffset = begin -> vertexInfo.size;
    size_t colorOffset = textureOffset + begin -> textureInfo.size;

    if (begin -> vertexInfo.n > 0) {
      glVertexPointer(begin -> vertexInfo.n, begin -> vertexInfo.type,
        stride, (char*)0 + vertexOffset);
      glEnableClientState(GL_VERTEX_ARRAY);
    }
    if (begin -> textureInfo.n > 0) {
      glTexCoordPointer(begin -> textureInfo.n, begin -> textureInfo.type,
        stride, (char*)0 + textureOffset);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (begin -> colorInfo.n > 0) {
      glColorPointer(begin -> colorInfo.n, begin -> colorInfo.type,
        stride, (char*)0 + colorOffset);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    glDrawArrays(mode, offset, (end - begin));

    if (begin -> colorInfo.n > 0) {
      glDisableClientState(GL_COLOR_ARRAY);
    }
    if (begin -> textureInfo.n > 0) {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (begin -> vertexInfo.n > 0) {
      glDisableClientState(GL_VERTEX_ARRAY);
    }
  }

  /** @brief Update the VBO, and draw it in the given @a mode.
   *
   *  See the other @a draw function for details.
   */
  template <class Container>
  void draw(const Container& data, GLenum mode, GLint offset = 0)
  {
    draw(data.begin(), data.end(), mode, offset);
  }

  /// Resize the VBO.
  void resize(size_t size);

  /// Return whether the VBO auto-resizes upon large updates.
  bool getAutoResize() const { return auto_resize_; }

  /// Set whether the VBO auto-resizes upon an update that would overflow.
  void setAutoResize(bool b) { auto_resize_ = b; }

 protected:
  GLuint          label_;
  size_t          size_;
  bool            auto_resize_;
};

#endif
