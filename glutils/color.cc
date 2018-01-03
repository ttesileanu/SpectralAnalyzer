#include "color.h"

const VboInfo GlColoredVertex2::vertexInfo(2, GL_FLOAT);
const VboInfo GlColoredVertex2::textureInfo(0, GL_FLOAT);
const VboInfo GlColoredVertex2::colorInfo(4, GL_FLOAT);

std::istream& operator>>(std::istream& in, GlColor4& c)
{
  // XXX this doesn't behave nice upon failure
  in >> c.r;
  int ch;
  do {
    ch = in.get();
  } while (std::isspace(ch));
  if (ch != ',') {
    in.unget();
    return in;
  }

  in >> c.g;
  do {
    ch = in.get();
  } while (std::isspace(ch));
  if (ch != ',') {
    in.unget();
    return in;
  }

  in >> c.b;
  do {
    ch = in.get();
  } while (std::isspace(ch));
  if (ch != ',') {
    in.unget();
    if (in.rdstate() == std::istream::eofbit)
      in.clear();

    return in;
  }

  in >> c.a;

  return in;
}
