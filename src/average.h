#pragma once

#include <Arduino.h>

#include "filter.h"

//
// This adc task takes the average of n samples.
//
class average : public filter
{

public:

  average(uint8_t n = 1);

  // Clear the filter read for a new sample set.
  // n is the number of samples to average over.
  void clear();

  // Return true if the average has been taken.
  bool finished() const { return finished_; }

  // Return the sample average.
  // Shuld only be called when finished.
  uint8_t value() const;

  // Accept and proces a new sample.
  // Called from the background adc isr.
  void process_sample(uint8_t sample) override;

private:

  // The number of samples to average.
  uint8_t n_ = 1;

  // The current number of samples taken.
  volatile uint8_t count_ = 0;

  // The running sample sum.
  volatile uint16_t sum_ = 0;

  // Will be true when the task is finished.
  volatile bool finished_ = false;
};