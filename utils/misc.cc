#include "utils/misc.h"

#include <boost/lexical_cast.hpp>

#include <sstream>

std::string trim(const std::string& s)
{
  size_t i = s.find_first_not_of(" \t\n\r");
  if (i == std::string::npos)
    return std::string();

  size_t j = s.find_last_not_of(" \t\n\r");

  return s.substr(i, j - i + 1);
}

StringVector splitString(const std::string& s, bool trim_ws)
{
  // XXX trim_ws not obeyed (always assumed to be true)!
  StringVector res;

  std::string crt = trim(s);

  do {
    size_t i = crt.find_first_of(" \t\n\r");
    if (i == std::string::npos)
      break;

    res.push_back(trim(crt.substr(0, i)));
    crt = crt.substr(i + 1);
  } while (true);

  if (!crt.empty())
    res.push_back(crt);

  return res;
}
