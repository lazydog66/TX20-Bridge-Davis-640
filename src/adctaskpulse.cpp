
#include <Arduino.h>

#include "adctaskpulse.h"

adctaskpulse::adctaskpulse(uint16_t period, uint8_t pulse_width, uint8_t debounce_width, uint8_t low_level, uint8_t pin)
    : adctask(pin), period_{period}, pulse_width_{pulse_width}, debounce_width_{debounce_width}, low_level_{low_level},
    moving_average_{pulse_width}
{
}

void adctaskpulse::start()
{
  cli();

  pulse_count_ = 0;
  debounce_count_ = debounce_width_;
  finished_ = false;
  moving_average_.clear();

  // Start this adc task.
  start_t_ = millis();
  start_task(this, adc_pin_);

  sei();
}

void adctaskpulse::stop()
{
  cli();

  finished_ = true;

  // Stop this adc task.
  stop_task(this);

  sei();
}

bool adctaskpulse::finished() const
{
  if (!finished_) finished_ = millis() - start_t_ >= period_;

  return finished_;
}

void adctaskpulse::service(uint8_t sample_value)
{
  // Nothing to do if finished.
  if (finished_) return;

  // Check if actually finished.
  finished_ = millis() - start_t_ >= period_;
  if (finished_) return;

  // Not finished yet, so process the sample through the moving average filter.
  // Note the moving average uses inverted logic.
  sample_value = sample_value <= low_level_ ? 1 : 0;
  moving_average_.push(sample_value);

  // Calculate the moving average, if it's equal to one then we've detected a pulse.
  const uint8_t sum = moving_average_.sum();
  const bool have_pulse = sum >= pulse_width_;

  // However, we may need to debounce the input.
  if (debounce_count_) --debounce_count_;

  if (have_pulse) {
    if (debounce_count_ == 0) ++pulse_count_;

    debounce_count_ = debounce_width_;
  }
}
