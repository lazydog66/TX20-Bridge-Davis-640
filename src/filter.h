#pragma once

#include <Arduino.h>

//
// This is the base class for the adc filters.
//

#pragma once

class filter
{

 public:
  filter() = default;
  virtual ~filter();

  // Clear the filter ready for a new sample set.
  virtual void clear() = 0;

  // Accept a sample and process the output.
  virtual void process_sample(uint8_t value) = 0;

  // Set the forwarding filter.
  // Once this filter has processed the sample, the sample is forwarded on/ to the next filter in the chain.
  // Note, the pointer becomes owned by the filter.s
  void set_forward_filter(filter* forward_filter);

 protected:
  // Send a sample value on to the forward filter.
  void forward_sample(uint8_t sample_value);

 private:
  // Once the filter has processed the sample, it is forwarded on to the next filter in the chain.
  filter* forward_filter_ = nullptr;
};