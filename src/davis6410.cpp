#include "davis6410.h"

#include <math.h>

using microseconds_t = unsigned long;
using milliseconds_t = unsigned long;

// Debounce period for the wind speed pulses.
// At 200 mph we have 1 pulse per 11.26 ms. With a realistic max wind speed of
// 130 mph we can expect a pulse every 17.32 ms. Therefore a debounce period of
// less than 17 ms should suffice.
constexpr milliseconds_t k_wind_pulse_debounce = 10;

// The counter for the wind pulses.
// The anenometer spins at 1600 rev/hrs at 1 mph, or 0.444r pulses per second
// per 1 mph. For a max speed of 200 mph this works out 88.888r pulses per
// second or 222.222r pulses in a 2.5 second sampling period. An 8 bit counter
// should be more than sufficient.
static volatile uint8_t wind_speed_pulse_counter = 0;

// At 200 mph we have 1 pulse per 11.26 ms. With a realistic max wind speed of
// 130 mph we can expect a pulse every 17.32 ms. Therefore a debounce period of
// less than 17 ms should suffice.
static volatile milliseconds_t debounce_start_t = 0;

// --------------------------------------------------------------------------------------------------------------------
// The isr for servicing the wind speed reading.
// The variable debounce_start_t should be cleared before the first interrupt of
// a sample period.
// --------------------------------------------------------------------------------------------------------------------
static void isr_6410() {
  milliseconds_t now = millis();
  if (now - debounce_start_t >= k_wind_pulse_debounce) {
    ++wind_speed_pulse_counter;
    debounce_start_t = now;
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Constructor does not initialise the hardware.
// --------------------------------------------------------------------------------------------------------------------
davis6410::davis6410(int wind_speed_pin, int wind_vane_pin,
                     unsigned long sample_period)
    : wind_speed_pin_{wind_speed_pin},
      wind_vane_pin_{wind_vane_pin},
      sample_period_{sample_period} {}

// --------------------------------------------------------------------------------------------------------------------
// The isr for servicing the wind speed reading.
// The variable debounce_start_t should be cleared before the first interrupt of
// a sample period.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::initialise() {
  pinMode(wind_speed_pin_, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(wind_speed_pin_), isr_6410, FALLING);

  state_ = idle;
  initialised_ = true;

  // Interrupts enabled.
  sei();
}

// --------------------------------------------------------------------------------------------------------------------
// Start a ne sample.
// The callback will be called when the sample is ready.
// --------------------------------------------------------------------------------------------------------------------
bool davis6410::start_sample(windsamplefn fn) {
  // Must be initialised and idle.
  if (!initialised_ || state_ != idle) return false;

  sample_fn_ = fn;
  state_ = new_sample;

  return true;
}

// --------------------------------------------------------------------------------------------------------------------
// Service the interface.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::service() {
  switch (state_) {
    case idle: {
      break;
    }

    case new_sample: {
      // Start a new sample off.
      wind_speed_pulse_counter = 0;
      sample_start_time_ = millis();

      state_ = sampling_speed;

      break;
    }

    case sampling_speed: {
      // Check if the sample frame has finished.
      if (millis() - sample_start_time_ >= sample_period_) {
        sample_pulse_count_ = wind_speed_pulse_counter;
        
        // Sample the wind direction.
        state_ = sampling_direction;
      }

      break;
    }

    case sampling_direction: {
      // Read the wind direction directly.
      sample_direction_ = analogRead(wind_vane_pin_);

      state_ = send_frame;

      break;
    }

    case send_frame: {
      // Ready for another sample.
      state_ = idle;

      // Let the client knowthe sampled wind speed and direction.
      sample_fn_(get_wind_mph(), get_wind_direction());

      break;
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Return the last sampled wind speed.
// The calcualtion from pulse count to mph uses the formula V=P(2.25/T). If we
// find that it is not accurate enough we could use calibration tables etc for
// greateer accuracy.
// --------------------------------------------------------------------------------------------------------------------
float davis6410::get_wind_mph() const {
  return sample_pulse_count_ * 2.25f * 1000.f / static_cast<float>(sample_period_);
}

// --------------------------------------------------------------------------------------------------------------------
// Return the last sampled wind direction.
// The calcualtio from pulse count to mph uses the formula V=P(2.25/T). If we
// find that it is not accurate enough we could use calibration tables etc for
// greateer accuracy.
// --------------------------------------------------------------------------------------------------------------------
int davis6410::get_wind_direction() const {
  return (sample_direction_ + 31) >> 6;
}
