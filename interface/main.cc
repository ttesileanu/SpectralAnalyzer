#include <iostream>

#include "interface/spectrum.h"
#include "utils/exception.h"
#include "utils/logging.h"
#include "utils/properties.h"

int main(int argc, char* argv[])
{
  // set up the logging framework
  const int verbosity = 1;
  const int log_verbosity = 1;
#ifdef NDEBUG
  const bool show_debug = false;
#else
  const bool show_debug = true;
#endif
  logger::init(verbosity, show_debug);
  logger::add_log_file("log.txt", log_verbosity, false);

  const std::string ini_name = "spectrum.xml";

  Properties parameters;

  // on Macs for some reason exceptions behave very stupidly: no useful details
  // are output about the exception type, and some dialog about sending crash
  // information to Apple pops up... To avoid that, we handle the exceptions
  // ourselves.

  try {
    // read the initialization file
    try {
      read_xml(ini_name, parameters, boost::property_tree::xml_parser::
        trim_whitespace);
    }
    catch (...) {
      std::cerr << "Error loading initialization file." << std::endl;
      return 1;
    }

    // create the app, and give it the settings
    SpectrumApp app;
    app.setProperties(&(parameters.get_child("settings")));

    app.execute();
  }
  catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch (const boost::bad_any_cast& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch (const boost::property_tree::ptree_error& e) {
    std::cerr << "Error reading settings file (" << e.what() << ")."
              << std::endl;
    return 1;
  }
  catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // if everything was fine, try to write out the configuration file
  try {
    my_write_xml(ini_name, parameters);
  }
  catch (const boost::property_tree::ptree_error& e) {
    std::cerr << "Error writing settings file (" << e.what() << ")."
              << std::endl;
    return 1;
  }

  return 0;
}
