#include "movingaverage.h"

movingaverage::movingaverage(uint8_t width) : width_{width} {
  // Note, this moving average starts off with a moving average of 0.
  clear();
}

void movingaverage::clear() {
  sum_ = 0;
  index_ = 0;

  for (uint16_t i = 0; i < width_; ++i) samples_[i] = 0;
}

uint8_t movingaverage::average() const { return sum_ / width_; }

void movingaverage::push(uint8_t value) {
  // Pop the oldest value and adjust the sum.
  sum_ -= samples_[index_];

  // Push the new value.
  samples_[index_] = value;
  index_ = (index_ + 1) % width_;

  // Adjust the sum.
  sum_ += value;
}
