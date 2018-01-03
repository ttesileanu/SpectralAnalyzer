#include "utils/logging.h"

#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

namespace logger {

dispatcher debug ("debug");
dispatcher detail ("detail");
dispatcher info ("info");
dispatcher error ("error");

void dispatcher::output_prefix_ ()
{
  if (prefix_time_) {
    helper_ << second_clock::local_time () << " ";
  }
  if (prefix_name_) {
    helper_ << "[" << name_ << "] ";
  }
}

streams& streams::instance ()
{
  static streams instance;

  return instance;
}

streams& streams::init (int verbosity, bool do_debug)
{
  streams& this_instance = instance ();

  this_instance.dispatchers_.push_back (&error);
  this_instance.dispatchers_.push_back (&info);
  this_instance.dispatchers_.push_back (&detail);
  if (do_debug)
    this_instance.dispatchers_.push_back (&debug);

  int max = verbosity + 2;
  if (max > (int)(this_instance.dispatchers_.size ()))
    max = this_instance.dispatchers_.size ();
  for (int i = 0; i < max; ++i) {
    if (i < 1)
      this_instance.dispatchers_[i] -> add_sink ("stderr", std::cerr);
    else
      this_instance.dispatchers_[i] -> add_sink ("stdout", std::cout);
  }

  return this_instance;
}

void streams::add_log_file (const std::string& name, int verbosity, bool append)
{
  streams& this_instance = instance ();

  std::ios_base::openmode mode = std::ios_base::out;
  if (append)
    mode |= std::ios_base::app;
  this_instance.logs_.push_back (new std::ofstream (name.c_str (), mode));
  std::ofstream* new_log = this_instance.logs_.back ();

  std::string log_name = "log" +
    boost::lexical_cast<std::string>(this_instance.logs_.size());

  int max = verbosity + 2;
  if (max > (int)this_instance.dispatchers_.size ())
    max = this_instance.dispatchers_.size ();
  for (int i = 0; i < max; ++i) {
    this_instance.dispatchers_[i] -> add_sink (log_name, *new_log);
  }
}

} // namespace log
