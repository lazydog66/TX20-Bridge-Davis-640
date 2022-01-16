#pragma once

#include <Arduino.h>

// This is the maximum width for the moving average class.
constexpr uint8_t k_moving_average_max_width = 128;

class movingaverage
{

public:
  // Construct a moving average filter with a defined width (measured in samples).
  movingaverage(uint8_t width);

  // Clear the moving average ready for another sample set.
  void clear();

  // Return the current value of the moving average.
  // Note, the moving average isn't valid until 'width' samples have been taken
 // uint8_t average() const;

  // Return the moving average sum.
  // Use this rather than the average because it avoids the division.
  uint8_t sum() const { return sum_; }

  // Push a new sample into the moving average.
  void push(uint8_t value);

private:
  // Width of the filter, in samples.
  uint8_t width_;

  // The current sum of the moving average.
  uint8_t sum_ = 0;

  // Index of the oldest sample, and also the index of the next insertion
  uint8_t index_ = 0;

  // Note, this moving average starts off with a moving average of 0.
  uint8_t samples_[k_moving_average_max_width];
};