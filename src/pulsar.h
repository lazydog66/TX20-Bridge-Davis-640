
// This filter looks for active low pulses.
//
// leadWidth - minimum lead time before a pulse, measure in samples count
// pulseWidth - minimimum width of the pulse, measured in samples count
// lowLevel - threshold for logic low, range [0, 255]
//
// debounce ->|<- min pulse width ->|
// ************                     **********
//            *                     *
//            *                     *
//            ***********************
//
#pragma once

#include <Arduino.h>

#include "filter.h"
#include "movingaverage.h"

// Definition for the pulse detection callback.
using pulsefn = void (*)();

class pulsar : public filter
{

 public:
  
  pulsar(uint8_t pulse_width, uint16_t debounce_width, uint8_t low_level, uint8_t noise_threshold, pulsefn fn);
  
  ~pulsar() = default;

  // Clear the filter ready for another run of pulses.
  void clear() override;

  // Process a sample value.
  // If the sample triggers the detection of a pulse, then the callback function is called.
  void process_sample(uint8_t value) override;

 private:
  // This is the minimum width for a pulse to be detected, measured in samples count.
  uint8_t pulse_width_ = 0;

  // Count the number of samples since the last pulse.
  // This is used to debounce the pulses.
  uint16_t debounce_width_ = 0;

  // This is the threshold value for a logic low, ie 0 when <= low_level_.
  uint8_t low_level_ = 0;

  // The pulse detector uses a moving average filter that has the same minimum width as the pulse.
  movingaverage moving_average_filter_ = 0;

  // This is the maximum value for the output of the moving average filter for a pulse to be detected.
  uint8_t moving_average_max_ = 0;

  // This is the sample index for the last detected pulse.
  // It's used for debouncing the pulses.
  uint8_t last_pulse_index_ = 0;

    // Count the number of samples since the last pulse.
  // This is used to debounce the pulses.
  uint16_t debounce_count_ = 0;

  // This is the callback for when pulses are detected.
  pulsefn fn_ = nullptr;
};
