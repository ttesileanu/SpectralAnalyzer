#include "properties.h"

#include <fstream>

// anonymous namespace
namespace {

void _write_xml(std::ofstream& out, const Properties& props, unsigned indent)
{
  std::string indentation(indent, ' ');

  // if there is a non-empty value, write it out
  std::string value = props.get_value<std::string>();
  if (!value.empty())
    out << indentation << value << std::endl;

  bool first = true;
  for (Properties::const_iterator i = props.begin(); i != props.end(); ++i) {
    const std::string& tag = i -> first;

    if (tag == "<xmlattr>") // this was already taken care of
      continue;

    if (!first) {
      out << std::endl;
    } else {
      first = false;
    }

    out << indentation;

    size_t n_attribs = 0;
    // is this a comment?
    if (tag == "<xmlcomment>") {
      out << "<!--";
      out << i -> second.get_value<std::string>();
      out << "-->";
    } else {
      out << "<" << i -> first;
      // are there any attributes?
      Properties::const_assoc_iterator j = i -> second.find("<xmlattr>");
      if (j != i -> second.not_found()) {
        n_attribs = 1;
        const Properties& attrs = j -> second;
        for (Properties::const_iterator k = attrs.begin();
              k != attrs.end();
              ++k)
        {
          out << " ";
          out << k -> first << "=\"" << k -> second.get_value<std::string>()
              << "\"";
        }
      }
      if (i -> second.size() == n_attribs &&
          i -> second.get_value<std::string>().empty()) {
        out << " />";
      } else {
        out << ">";

        if (i -> second.size() == n_attribs) {
          out << i -> second.get_value<std::string>();
        } else {
          out << std::endl;
          _write_xml(out, i -> second, indent + 2);
          out << std::endl << indentation;
        }

        out << "</" << i -> first << ">";
      }
    }
  }
}

} // anonymous namespace

void my_write_xml(const std::string& fname, const Properties& props)
{
  std::ofstream f(fname.c_str());
  f << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
  _write_xml(f, props, 0);
  f << std::endl;
}
