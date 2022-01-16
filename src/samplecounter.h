#pragma once

#include "filter.h"

//
// This filter simply counts the number of samples.
//
class samplecounter : public filter
{

 public:
  samplecounter(uint16_t period);

  // Start counting.
  void clear();

  // Return whether the coutner has finished.
  bool finished() const;

  // Return the current sample count.
  uint8_t value() const { return count_; }

  // Accept and proces a new sample.
  // Simply increments the running count.
  void process_sample(uint8_t sample);
  
 private:
  // The coutning period in milliseconds.
  unsigned long period_ = 0;

  // The counter for the number of samples that have arrived.
  volatile uint8_t count_ = 0;

  // This wil lbe set to true when the counting period has ended.
  mutable bool finished_ = true;

  // The start time for the current session..
  unsigned long start_t_ = 0;
};