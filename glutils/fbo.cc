#include "fbo.h"

Fbo::Stack Fbo::stack_;
Fbo*  Fbo::current_ = 0;

void Fbo::associate(const TextureSharedPtr& tex)
{
  bind();
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
    GL_TEXTURE_2D, tex -> getLabel(), 0);

  tex_ = tex;
}

void Fbo::pop()
{
  if (stack_.empty()) {
    unbind();
  } else {
    if (stack_.back())
      stack_.back() -> bind();
    else
      unbind();

    stack_.pop_back();
  }
}

Fbo::TextureSharedPtr Fbo::newTex_(unsigned width, unsigned height) const
{
  TextureSharedPtr res(new Texture(width, height));
  res -> bind();
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return res;
}
