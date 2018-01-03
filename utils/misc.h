/** @file misc.h
 *  @brief Defines various miscellaneous functions.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef MISC_H_
#define MISC_H_

#include <vector>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

/// A class for measuring durations of processes.
class Timer {
 public:
  /// Constructor.
  Timer() { reset(); }

  /// Reset the timer.
  void reset() {
    start_ = boost::posix_time::microsec_clock::universal_time();
  }
  /// Get the elapsed time as a @a posix_time::time_duration.
  boost::posix_time::time_duration getElapsedPtime() const {
    return boost::posix_time::microsec_clock::universal_time() - start_;
  }
  /// Get the elapsed time in microseconds.
  long getElapsedMicro() const {
    return getElapsedPtime().total_microseconds();
  }
  /// Get the elapsed time in seconds.
  double getElapsed () const {
    return static_cast<double>(getElapsedMicro()) / 1000000.0;
  }

 private:
  boost::posix_time::ptime	start_;
};

/// Trim the whitespace from a string.
std::string trim(const std::string& s);

/// A vector of strings.
typedef std::vector<std::string> StringVector;

/// Split a space-separated string into a vector of strings.
StringVector splitString(const std::string& s, bool trim_ws = true);

#endif // MISC_H_
