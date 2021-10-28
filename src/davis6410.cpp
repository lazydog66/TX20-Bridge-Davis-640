// ------------------------------------------------------------------------------------------------
// This is a controller class for interfacing to a Davis6410 anemometer and wind vane.
// The interface requires one digitial io pin (capable of falling edge interrupts) and
// an analog pin. The anemometer is read by counting pulses on the io pin and converting
// the number of pulses per second to mph. The wind vane is read by reading the voltage on
// the analog pin and mapping the value to 16 wind directions.
// ------------------------------------------------------------------------------------------------
#include "davis6410.h"

#include <math.h>

using microseconds_t = unsigned long;
using milliseconds_t = unsigned long;

// The counter for the wind pulses.
// The anenometer spins at 1600 rev/hrs at 1 mph, or 0.444r pulses per second
// per 1 mph. This means an 8 bit counter should easily suffice for our needs.
// Using an 8 bit counter has the advantage that we dont need to disable interrutps
// when reading or clearing the counter.
static volatile uint8_t wind_speed_pulse_counter = 0;

// This variable is needed to debounce the reed switch.
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
// The variable debounce_start_t should be cleared before the first interrupt of a sample period.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::initialise() {
  pinMode(wind_speed_pin_, INPUT);
  attachInterrupt(digitalPinToInterrupt(wind_speed_pin_), isr_6410, FALLING);

  state_ = davis6410state::idle;
  initialised_ = true;

  // Interrupts enabled.
  sei();
}

// --------------------------------------------------------------------------------------------------------------------
// Start a new sample.
// The callback will be called when the sample is ready.
// --------------------------------------------------------------------------------------------------------------------
bool davis6410::start_sample(windsamplefn fn, void* context) {
  // Must be initialised and idle.
  if (!initialised_ || state_ != davis6410state::idle) return false;

  sample_fn_ = fn;
  context_ = context;

  state_ = davis6410state::new_sample;

  return true;
}

// --------------------------------------------------------------------------------------------------------------------
// Abort the current sample if there is one in progress.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::abort_sample() {
  // Must be initialised.
  if (!initialised_) return;

  switch (state_) {

    case davis6410state::idle: break;

    case davis6410state::new_sample:
    case davis6410state::sampling_speed:
    case davis6410state::sampling_direction:
    case davis6410state::send_frame: {
      sample_fn_ = nullptr;
      state_ = davis6410state::idle;
      break;
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Service the interface.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::service() {
  switch (state_) {
    case davis6410state::idle: {
      break;
    }

    case davis6410state::new_sample: {
      // Start a new sample off.
      wind_speed_pulse_counter = 0;
      sample_start_time_ = millis();

      state_ = davis6410state::sampling_speed;

      break;
    }

    case davis6410state::sampling_speed: {
      // Check if the sample frame has finished.
      if (millis() - sample_start_time_ >= sample_period_) {
        sample_pulse_count_ = wind_speed_pulse_counter;

        // Sample the wind direction.
        state_ = davis6410state::sampling_direction;
      }

      break;
    }

    case davis6410state::sampling_direction: {
      // Read the wind direction directly.
      sample_direction_ = analogRead(wind_vane_pin_);

      state_ = davis6410state::send_frame;

      break;
    }

    case davis6410state::send_frame: {
      // Ready for another sample.
      state_ = davis6410state::idle;

      // Let the client know the sampled wind speed and direction.
      if (sample_fn_) sample_fn_(context_);

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
  return sample_pulse_count_ * 2.25f * 1000.f /
         static_cast<float>(sample_period_);
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

// --------------------------------------------------------------------------------------------------------------------
// Return the last sampled anenometer pulse count.
// Each pulse is one revolution of the wind cups.
// --------------------------------------------------------------------------------------------------------------------
uint8_t davis6410::get_pulses() const {
  return sample_pulse_count_;
}

