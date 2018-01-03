/** @file logging.h
 *  @brief Defines a simple logger interface.
 *
 *  The output is directed to a set of @a dispatcher objects, which behave
 *  like STL @a streams. Each of the @a dispatchers outputs each line to
 *  several @a sinks, prepending a time and a string identifier for the
 *  @a dispatcher.
 *
 *  An initializer function and a function taking integer parameters are used
 *  to control output levels given a @a verbosity level. Output to several
 *  log files is also supported.
 *
 *  @author Tiberiu Tesileanu
 */

#ifndef UTILS_LOGGING_H_
#define UTILS_LOGGING_H_

#include <ostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

/// All logging functions are included in this namespace.
namespace logger {

/** @brief This is an internal class. It shouldn't be instantiated directly.
 *
 *  This is just a thin wrapper that allows simultaneous output to several
 *  sinks.
 */
class dispatcher_helper {
 public:
  /// A sink.
  typedef std::ostream* sink;
  /// A series of sinks, with names given by strings.
  typedef std::map<std::string, sink> sinks;

  /// Add a new sink.
  void add_sink (const std::string& name, std::ostream& stream)
    { sinks_[name] = &stream; }
  /// Remove a sink.
  void remove_sink (const std::string& name)
    { sinks_.erase (name); }

  /// Handle output.
  template <class T> friend dispatcher_helper& operator<<
      (dispatcher_helper& str, T msg);

  /// Handle @a std::stream manipulators.
  dispatcher_helper& operator<< (std::ostream& (*pf)(std::ostream&)) {
    for (sinks::iterator i = sinks_.begin (); i != sinks_.end (); ++i) {
      *(i -> second) << pf;
    }
    return *this;
  }

 private:
  sinks       sinks_;
};

/** @brief This allows dispatching error messages to several sinks.
 *
 *  By default, every new stream output operation also outputs a prefix
 *  including the current time and the name of the dispatcher. This can
 *  be inhibited by using the @a cont manipulator. Also, manipulators
 *  in general do not trigger the output of the prefix.
 *
 *  @see cont
 */
class dispatcher {
 public:
  /// Constructor, taking the name of the newly created dispatcher.
  dispatcher (const std::string& name) : name_ (name), prefix_time_ (true),
      prefix_name_ (true) {}

  /// Handle output.
  template <class T> friend dispatcher_helper&
      operator<< (dispatcher& str, T msg);
  /// Handle @a std::stream manipulators.
  dispatcher_helper& operator<< (std::ostream& (*pf)(std::ostream&))
    { helper_ << pf; return helper_; }
  /// Handle manipulators written specifically for @a dispatcher.
  dispatcher_helper& operator<< (dispatcher_helper& (*pf)(dispatcher&))
    { return pf (*this); }

  /// @see cont
  friend dispatcher_helper& cont (dispatcher&);

  /// Add a new sink.
  void add_sink (const std::string& name, std::ostream& stream)
    { helper_.add_sink (name, stream); }
  /// Remove a sink.
  void remove_sink (const std::string& name)
    { helper_.remove_sink (name); }

 private:
  void output_prefix_ ();

  dispatcher_helper     helper_;
  std::string           name_;
  bool                  prefix_time_;
  bool                  prefix_name_;
};

/** @brief A manipulator used to inhibit the output of the prefix. 
 *
 *  By default each new output operation on the @a dispatcher generates
 *  the output of a prefix. Note that this only includes instances where
 *  the @a << operator is applied directly on the @a dispatcher object, and
 *  excluding manipulators. So, for example, each of the following lines of
 *  code generate exactly one prefix:
 *  @code
 *    logger::info << "This is a message.";
 *    logger::info << "This is a message." << std::endl;
 *    logger::info << "This is a " << "longer message" << std::endl;
 *  @endcode
 *
 *  This manipulator allows to skip the prefix, so that the following coe
 *  also outputs the prefix only once:
 *  @code
 *    logger::info << "This ";
 *    logger::info << logger::cont << "is a message." << std::endl;
 *  @endcode
 */
inline dispatcher_helper& cont (dispatcher& out)
{
  return out.helper_;
}

/// The stream for debug output. By default this is sent to @a stdout.
extern dispatcher debug;
/// The stream for detailed output. By default this is sent to @a stdout.
extern dispatcher detail;
/// The stream for general info output. By default this goes to @a stdout.
extern dispatcher info;
/// The stream for error output. By default this goes to @a stderr.
extern dispatcher error;

/// Singleton class to handle all the output.
class streams {
 public:
  /// A vector of @a dispatchers.
  typedef std::vector<dispatcher*> dispatchers;

  /// Destructor.
  ~streams () { close_logs_ (); }

  /// Access the unique instance of the class.
  static streams& instance ();
  /** @brief Initialize the instance.
   *
   *  @param verbosity Controls how verbose the output should be. A value of 0
   *  means output @a error and @a info streams. A value of 1 also includes
   *  @a detail, and 2 or larger includes @a debug, provided @a do_debug is
   *  @a true. A value of -1 means only output @a error, and a value smaller
   *  than -1 means suppress all output.
   *  @param do_debug Controls whether to have a debug output. If @a do_debug
   *  is @a false, then output to the @a debug stream is ignored.
   */
  static streams& init (int verbosity, bool do_debug);

  /** @brief Add output to a log file.
   *
   *  @param name Log file name.
   *  @param verbosity Verbosity level for file output.
   *  @param append Whether to append to log file, or replace.
   */
  static void add_log_file (const std::string& name, int verbosity,
    bool append);

  /** @brief Access one of the dispatchers.
   *
   *  0 = @a error
   *  1 = @a info
   *  2 = @a detail
   *  3 = @a debug
   */
  static dispatcher& at (int i) {
    if (i < 0 || i >= (int)instance ().dispatchers_.size())
      throw std::out_of_range("[log::out()] invalid dispatcher.");
    return *(instance ().dispatchers_[(size_t)i]);
  }

  /// Get access to the vector of dispatchers.
  dispatchers& get_dispatchers () const { return instance ().dispatchers_; }

 private:
  void close_logs_ () {
    for (std::vector<std::ofstream*>::iterator i = logs_.begin ();
          i != logs_.end ();
          ++i)
    {
      delete *i;
    }
  }

  dispatchers                 dispatchers_;
  std::vector<std::ofstream*> logs_;

  streams () {}
  streams (const streams&) {}
  void operator=(const streams&) {}
};

/** @brief Access a dispatcher corresponding to the given verbosity level.
 *
 *  -1 = @a error
 *   0 = @a info
 *   1 = @a detail
 *   2 = @a debug
 */
inline dispatcher& out (int i)
{
  return streams::at (i + 1);
}

/** @brief Initialize the output.
 *
 *  @see streams::init
 */
inline streams& init (int verbosity, bool do_debug)
{
  return streams::init (verbosity, do_debug);
}

/** @brief Add a new log file.
 *
 *  @see streams::add_log_file
 */
inline void add_log_file (const std::string& name, int verbosity,
    bool append = true)
{
  streams::add_log_file (name, verbosity, append);
}

/// Handle data output to a @a dispatcher_helper.
template <class T>
dispatcher_helper& operator<< (dispatcher_helper& str, T msg)
{
  for (dispatcher_helper::sinks::iterator i = str.sinks_.begin ();
        i != str.sinks_.end (); ++i) {
    *(i -> second) << msg;
  }
  return str;
}

/// Handle data output to a @a dispatcher.
template <class T>
dispatcher_helper& operator<< (dispatcher& str, T msg)
{
  str.output_prefix_ ();
  str.helper_ << msg;
  return str.helper_;
}

} // namespace log

#endif // UTILS_LOGGING_H_
