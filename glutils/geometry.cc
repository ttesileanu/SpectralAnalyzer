#include "geometry.h"

const VboInfo GlVertex2::vertexInfo(2, GL_FLOAT);
const VboInfo GlVertex2::textureInfo(0, GL_FLOAT);
const VboInfo GlVertex2::colorInfo(0, GL_FLOAT);
const VboInfo GlVertexTex2::vertexInfo(2, GL_FLOAT);
const VboInfo GlVertexTex2::textureInfo(2, GL_FLOAT);
const VboInfo GlVertexTex2::colorInfo(0, GL_FLOAT);

std::istream& operator>>(std::istream& in, GlVertex2& v)
{
  // XXX this doesn't behave nice upon failure
  in >> v.x;
  int c;
  do {
    c = in.get();
  } while (std::isspace(c));
  if (c == ',') {
    in >> v.y;
  } else {
    in.unget();
  }

  return in;
}
