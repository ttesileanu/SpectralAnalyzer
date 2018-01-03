/** @file fbo.h
 *  @brief Defines an RAII wrapper class for framebuffer objects (FBOs).
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GLUTILS_FBO_H_
#define GLUTILS_FBO_H_

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "glutils/texture.h"
#include "glutils/gl_incs.h"

// XXX check that the FBO extension is present!
/** @brief An RAII wrapper for FBOs.
 *
 *  This class can be used to manage the creation, destruction, binding, and
 *  unbinding of the framebuffer objects, as well as their association with
 *  textures. One extra feature that is available provided all FBO binding is
 *  done using this class, is a stack for FBOs, allowing to push/pop the
 *  framebuffer state.
 */
class Fbo : boost::noncopyable {
 public:
  typedef boost::shared_ptr<Texture> TextureSharedPtr;
  typedef boost::shared_ptr<const Texture> TextureConstSharedPtr;

  /// Create a new FBO.
  Fbo() { generate_(); }
  /** @brief Create a new FBO and associate it with the given texture.
   *
   *  This binds both the FBO and the texture.
   */
  explicit Fbo(const TextureSharedPtr& tex) { generate_(); associate(tex); }
  /** @brief Create a new FBO, and associate it with a texture of the given
             size.
   *
   *  This binds both the FBO and the texture.
   */
  Fbo(unsigned width, unsigned height)
    { generate_(); associate(newTex_(width, height)); }

  /// Destroy the FBO. Note that this also unbinds any FBO!
  ~Fbo() { unbind(); glDeleteFramebuffersEXT(1, &label_); }

  /// Get access to the texture.
  const TextureSharedPtr& getTexture() { return tex_; }
  /// Get read-only access to the texture.
  TextureConstSharedPtr getTexture() const { return tex_; }

  /// Associate a new texture with the FBO.
  void associate(const TextureSharedPtr& tex);

  /// Bind the FBO. Does nothing if the FBO is on the top of the stack.
  void bind()
    { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, label_); current_ = this; }

  /// Push the current FBO to the stack.
  static void push() { stack_.push_back(current_); }

  /** @brief Unbind the current FBO, and pop the one on the top of the stack in
             its place.
   *
   *  If there are no FBOs in the stack, call @a unbind().
   */
  static void pop();

  /// Unbind any FBO.
  static void unbind()
    { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); current_ = 0; }

  /// Get the integer label for this FBO.
  GLuint getLabel() const { return label_; }

 protected:
  typedef std::vector<Fbo*> Stack;

  void generate_() { glGenFramebuffersEXT(1, &label_); }
  TextureSharedPtr newTex_(unsigned width, unsigned height) const;

  GLuint            label_;
  TextureSharedPtr  tex_;

  static Stack      stack_;
  static Fbo*       current_;
};

#endif
