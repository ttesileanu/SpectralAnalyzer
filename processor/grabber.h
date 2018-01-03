/** @file grabber.h
 *  @brief Defines a processing module that just grabs and stores the input
 *  from an input back end.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef GRABBER_H_
#define GRABBER_H_

#include <vector>

#include "processor/base_processor.h"
#include "input/base_input.h"

/** @brief A processing module that just grabs and stores data from an input
 *  back end.
 *
 *  This should have no usual BaseProcessor inputs; instead, assign an input
 *  back end by using assignBackend.
 */
class Grabber : public BaseProcessor {
 public:
  struct DetailsStruct {
    float           samplingFrequency;
    unsigned        size;
  };
  typedef const DetailsStruct* Details;
  typedef const std::vector<float>* Output;

  /// Assign a backend to the grabber.
  void assignBackend(BaseInput* input) { backend_ = input; }

 protected:
  /// Implementation of the grabbing.
  virtual int execute();

  /// Get access to the data.
  virtual boost::any getOutput_() const { return &data_; }

  virtual boost::any getDetails_() const { return &details_; }

 private:
  std::vector<float>      data_;
  BaseInput*              backend_;
  DetailsStruct           details_;
};

#endif
