#pragma once

#include <Arduino.h>

#include "adctask.h"

//
// This adc task takes the average of n samples.
//
class adctaskaverage : public adctask
{

public:
  adctaskaverage(uint8_t adc_pin);

  // Start a new convertion.
  // n is the number of samples to average over.
  void start(uint8_t n = 1);

  // Return true if the caverage has been taken.
  bool finished() const { return finished_; }

  // Return the sample average.
  // Shuld only be called when finished.
  uint8_t average() const;

private:
  // Accept and proces a new sample.
  // Called from the background adc isr.
  void service(uint8_t sample) override;

  // The number of samples to average.
  uint8_t n_ = 1;

  // The current number of samples taken.
  volatile uint8_t count_ = 0;

  // The running sample sum.
  volatile uint16_t sum_ = 0;

  // Ignore the first few samples.
  // This is because when starting a new average, the next sample may not
  // be accurate for various reasons, eg channel changed.
  volatile uint8_t ignore_ = 0;

  // Will be true when the task is finished.
  volatile bool finished_ = false;
};