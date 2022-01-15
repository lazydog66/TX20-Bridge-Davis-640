#include <Arduino.h>

#include "pulsar.h"

pulsar::pulsar(uint8_t pulse_width, uint16_t debounce_width, uint8_t low_level, uint8_t noise_threshold, pulsefn fn)
    : pulse_width_{pulse_width}, debounce_width_{debounce_width},
      low_level_{low_level},
      moving_average_filter_{pulse_width},
      moving_average_max_{noise_threshold},
      fn_{fn}
{
}

//
// Clear the filter ready for a new session.
//
void pulsar::clear()
{
  debounce_count_ = 0;
  moving_average_filter_.clear();
}

//
// Accept a new sample.
//
void pulsar::process_sample(uint8_t value)
{
  if (debounce_count_)
    --debounce_count_;

  // Clean up the sample value.
  // Note the moving average uses inverted logic.
  value = value <= low_level_ ? 1 : 0;

  // Add this sample into the moving average buffer.
  moving_average_filter_.push(value);

  // Calculate the moving average, if it's equal to one then we've detected a pulse.
  const uint8_t average = moving_average_filter_.average();
  const bool have_pulse = average >= moving_average_max_;

  if (have_pulse)
  {
    if (debounce_count_ == 0 && fn_)
        fn_();

      debounce_count_ = debounce_width_;
  }
}
