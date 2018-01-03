#include "transition_store.h"

#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "animation/standard_easing.h"
#include "utils/exception.h"
#include "utils/misc.h"

void TransitionStore::setProperties(Properties* props)
{
  properties_ = props;

  for (Properties::const_iterator i = properties_ -> begin();
        i != properties_ -> end();
        ++i)
  {
    const std::string& name = i -> first;
    // skip comments
    if (name == "<xmlcomment>")
      continue;

    float len = i -> second.get<float>("<xmlattr>.length");
    std::string def = trim(i -> second.get_value<std::string>());
    std::string type_str, subtype_str, type_params_str;
    size_t pos_comma = def.find(',');
    if (pos_comma != std::string::npos) {
      type_str = trim(def.substr(0, pos_comma));
      subtype_str = trim(def.substr(pos_comma + 1));

      size_t pos_paren1 = type_str.find('(');
      size_t pos_paren2 = type_str.find(')');
      if (pos_paren1 != std::string::npos && pos_paren2 != std::string::npos) {
        if (pos_paren2 + 1 != type_str.size())
          throw Exception("Malformed transition definition for " + name + ": '"
            + def + "' (TransitionStore::setProperties).");
        type_str = trim(type_str.substr(pos_paren1));
        type_params_str = trim(type_str.substr(pos_paren1 + 1,
          pos_paren2 - pos_paren1 - 1));
      }
    } else {
      type_str = def;
      subtype_str = "in";
    }

    StandardEasing::Type type;
    StandardEasing::Subtype subtype;
    if (type_str == "none")
      type = StandardEasing::NONE;
    else if (type_str == "linear")
      type = StandardEasing::LINEAR;
    else if (type_str == "quadratic")
      type = StandardEasing::QUADRATIC;
    else if (type_str == "power")
      type = StandardEasing::POWER;
    else if (type_str == "sine")
      type = StandardEasing::SINE;
    else
      throw Exception("Unknown transition type for " + name + "'" + type_str
        + "' (TransitionStore::setProperties).");

    if (subtype_str == "in")
      subtype = StandardEasing::IN;
    else if (subtype_str == "out")
      subtype = StandardEasing::OUT;
    else if (subtype_str == "inout")
      subtype = StandardEasing::INOUT;
    else if (subtype_str == "outin")
      subtype = StandardEasing::OUTIN;
    else
      throw Exception("Unknown transition subtype for " + name + "'" +
        subtype_str + "' (TransitionStore::setProperties).");

    StandardEasing* easing = new StandardEasing(type, subtype);
    BaseEasingPtr easing_ptr(easing);
    if (!type_params_str.empty()) {
      std::vector<float> params;
      std::string crt;
      // to make it easy to parse the last parameter
      type_params_str = type_params_str + ",";
      for (size_t i = 0; i < type_params_str.length(); ++i) {
        char c = type_params_str[i];
        if (c == ',') {
          params.push_back(boost::lexical_cast<float>(crt));
          crt.clear();
        } else {
          crt = crt + c;
        }
      }
      switch (type) {
        case StandardEasing::NONE:
        case StandardEasing::LINEAR:
          if (!params.empty())
            throw Exception("Parameters for linear or no transition for "
              + name + ".");
          break;
        case StandardEasing::QUADRATIC:
          if (!params.empty()) {
            if (params.size() > 1)
              throw Exception("Too many parameters for quadratic transition "
                + name + ".");
            easing -> setStraightness(params[0]);
          }
          break;
        case StandardEasing::POWER:
          if (!params.empty()) {
            if (params.size() > 3)
              throw Exception("Too many parameters for power transition "
                + name + ".");
            easing -> setPower(params[0]);
            if (params.size() > 1) {
              easing -> setSubpower(params[1]);
              if (params.size() > 2) {
                easing -> setStraightness(params[2]);
              }
            }
          }
          break;
        case StandardEasing::SINE:
          if (!params.empty()) {
            if (params.size() > 1)
              throw Exception("Too many parameters for sine transition "
                + name + ".");
            easing -> setFrequency(params[0]);
          }
          break;
        default:;
      }
    }

    add(name, len, easing_ptr);
  }
}
