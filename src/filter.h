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
  virtual ~filter() = default;
  
  // Clear the filter ready for a new sample set.
  virtual void clear() = 0;

  // Accept a sample and process the output.
  virtual void process_sample(uint8_t value) = 0;

};