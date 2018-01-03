/** @file base_processor.h
 *  @brief Defines the interface for a signal processor. This uses data from
 *  some input module (e.g., audio stream) and processes it for use as input
 *  in a different processor/display module
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef BASE_PROCESSOR_H_
#define BASE_PROCESSOR_H_

#include <map>
#include <string>

#include <boost/any.hpp>

#include "utils/properties.h"

/** @brief This class defines the interface for a signal processor.
 *
 *  A signal processor is a module that takes in buffered input and outputs
 *  processed data. The type of the processed data can vary, as it is returned
 *  in a boost::any. Descendants should have a subtype called Output 
 *  describing the output data type.
 *
 *  The processor can also return more details about itself and the data it
 *  produces in another boost::any returned by getDetails. Descendants should
 *  define the subtype Details describing the output of getDetails.
 *
 *  Descendants can choose to ignore inputs, in which case they are input
 *  modules, ignore or outputs, in which case they are display modules.
 *  Examples of signal processors are a module implementing a certain windowing
 *  function, an input module, a module performing a FFT, a display module, or
 *  a module calculating the most intense frequency in the input stream.
 */
class BaseProcessor {
 public:
  /// Virtual destructor, for inheritance.
  virtual ~BaseProcessor() {}

  /// Add an input to the processor.
  void addInput(const std::string& name, BaseProcessor* input)
    { inputs_[name] = input; }

  /// Get the output of the processor.
  boost::any getOutput() { validate(); return getOutput_(); }

  /// Get details about the processor.
  boost::any getDetails() { validate(); return getDetails_(); }

  /** @brief Mark the cached values as invalid.
   *
   *  This is called once for every display cycle, to indicate the fact that
   *  the input data has been updated, and thus any cached values need to be
   *  recalculated.
   */
  void invalidateCache() { valid_ = false; }

  /// Give the module its settings.
  void setProperties(Properties* props) { properties_ = props; }

  /// Update the settings. Descendants should implement this. @see setProperties
  virtual void updateProperties() {}

 protected:
  typedef std::map<std::string, BaseProcessor*> Inputs;

  /** @brief Do the processing. Return zero if successful, non-zero otherwise.
   *
   *  This is to be implemented by descendants. These can use any of the
   *  other processors in the @a processors list to do their job. They can also
   *  use the input module.
   */
  virtual int execute() = 0;

  /// Get the output of the processor -- should be implemented by descendants.
  virtual boost::any getOutput_() const = 0;

  /// Get details about the processor -- descendants can override this.
  virtual boost::any getDetails_() const { return boost::any(); }

  BaseProcessor() : valid_(false) {}

  // check whether the cache is valid
  bool isValid() const { return valid_; }

  // mark the cache as valid
  void markValid() { valid_ = true; }

  // make sure the module has been executed
  void validate() { if (~isValid()) execute(); }

  Properties*           properties_;
  Inputs                inputs_;

 private:
  bool                  valid_;
};

#endif
