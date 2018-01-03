#include "texture.h"

Texture::Texture(unsigned width, unsigned height)
{
  generate_();
  bind();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, 0);
}
