#pragma once

#include <Arduino.h>

#include "adctask.h"


// This is the maximum width for the moving average class.
constexpr uint8_t k_moving_average_max_width = 255;

// Callback for the pulse detector.
using pulsedetectedfn = void (*)();

//
// This class is a moving average filter.
//
// Signals are filtered by pushing each sample through the filter.
// At any time, the moving average can be read back, though the first few
// filtered values should be ignored.
//
class movingaverage {
  //
  // Construct a moving average filter with a defined width (measured in
  // samples).
  //
  movingaverage(width) {
    // How many samples make up the moving average
    this.width = width;

    // The current sum of the samples.
    this.sum = 0;

    // Index of the oldest sample, and also the index of the next insertion
    this.index = 0;

    // Note, this moving average starts off with a moving average of 0.
    this.samples = Array.apply(null, Array(width)).map(() = > 0);
  }

  // Clear the moving average ready for another sample set.
  clear() {
    this.sum = 0;
    this.index = 0;
    this.samples = this.samples.map(() = > 0);
  }

  //
  // Return the current value of the moving average.
  // Note, the moving average isn't valid until 'width' samples have been taken
  average() { return this.sum / this.width; }

  //
  // Push a new sample into the moving average.
  //
  void push(uint8_t value) {
    // Pop the oldest value and adjust the sum.
    this.sum -= this.samples[this.index];

    // Push the new value.
    this.samples[this.index] = value;
    this.index = (this.index + 1) % this.width;

    // Adjust the sum.
    this.sum += value;
  }

 private:
  // Width of the filter, in samples.
  uint8_t width_;

  // The current sum of the moving average.
  uint16_t sum_ = 0;

  // Index of the oldest sample, and also the index of the next insertion
  uint8_t index_ = 0;

  // Note, this moving average starts off with a moving average of 0.
  uint8_t samples_[k_moving_average_max_width];
};

//
// This adc task looks for pulses (active low) with a minimum width.
//
class adctaskpulse : public adctask {
 public:
  adctaskpulse(uint8_t pulse_width_ms, uint8_t adc_pin);

  // Start the pulse detector.
  // n is the number of samples to average over.
  void start(pulsedetectedfn fn);

 private:
  // Accept and proces a new sample.
  // Called from the background adc isr.
  void service(uint8_t sample) override;

  // Ignore the first few samples.
  // This is because when starting a new average, the next sample may not
  // be accurate for various reasons, eg channel changed.
  volatile uint8_t ignore_ = 0;

  // Will be true when the task is finished.
  volatile bool finished_ = false;
};