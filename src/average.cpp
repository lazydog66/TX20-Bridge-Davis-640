#include "average.h"

// How many samples to igore at the srar of a new average.
constexpr uint8_t k_ignore_samples_count = 8;

average::average(uint8_t n)
 : filter{},
 n_{n}
  {}

void average::clear()
{

  finished_ = true;

  count_ = 0;
  sum_ = 0;
  finished_ = 0;
}

uint8_t average::value() const
{
  // TODO Improve by rounding.
  return sum_ / count_;
}

void average::process_sample(uint8_t sample)
{
  // If enough samples have already been taken, then there is nothing to do.
  if (finished_)
    return;

  // Update the running sum and check if finished.
  sum_ += sample;
  finished_ = ++count_ == n_;
}