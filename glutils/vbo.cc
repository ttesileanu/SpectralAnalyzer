#include "vbo.h"

Vbo::Vbo(unsigned size) : label_(0), size_(0), auto_resize_(1)
{
  glGenBuffers(1, &label_);
  bind();
  resize(size);
}

Vbo::~Vbo()
{
  unbind();
  glDeleteBuffers(1, &label_);
}

void Vbo::resize(size_t size)
{
  bind();
  // XXX make GL_DYNAMIC_DRAW configurable?
  glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
  size_ = size;
}
