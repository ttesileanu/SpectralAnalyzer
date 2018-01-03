/** @file exception.h
 *  @brief Defines the base class from which @a spectrum exceptions inherit.
 *
 *  Having a base class like this helps to filter @a spectrum exceptions from
 *  other types of exceptions.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <stdexcept>
#include <string>

/// All exceptions related to @a spectrum inherit from this.
class Exception : public std::runtime_error {
 public:
  /// Constructor, with text.
  explicit Exception(const std::string& what) : runtime_error(what) {}

  /// Virtual destructor, to allow for proper inheritance.
  virtual ~Exception() throw() {}
};

#endif
