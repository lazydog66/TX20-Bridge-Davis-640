// This is a controller class for interfacing to a Davis6410 anemometer and wind vane.
// The interface requires one digitial io pin (capable of falling edge interrupts) and
// an analog pin. The anemometer is read by counting pulses on the io pin and converting
// the number of pulses per second to mph. The wind vane is read by reading the voltage on
// the analog pin and mapping the value to 16 wind directions.

#include <Arduino.h>
#include <math.h>

#include "davis6410.h"
#include "fallingedgetask.h"
#include "adctask.h"
#include "samplecounter.h"
#include "average.h"
#include "pulsar.h"

using microseconds_t = unsigned long;
using milliseconds_t = unsigned long;

davis6410::davis6410(davis6410method method, int wind_speed_pin, int wind_vane_pin, unsigned long sample_period)
    : wind_speed_pin_{wind_speed_pin}, wind_vane_pin_{wind_vane_pin}, sample_period_{sample_period}
{
  // Create the task for reading the wind direction.
  // The wind direction is read by reading the analog value on the wind vane pin.
  wind_direction_task_ = new adctask(new average(k_wind_direction_average_n), wind_vane_pin);

  // The pulses on the wind speed line are counted as they arrive.
  // The pulse rate defines the measured wind speed.
  wind_pulse_counter_ = new samplecounter(k_wind_speed_sample_t);

  // Create the task for reading the wind speed.
  // There are two alternative ways of reading the wind speed, adc and falling edge interrupts.
  switch (method) {
    // The adc method requires an adc task and the pulsar filter.
    case davis6410method::adc: {
      filter *sample_filter = new pulsar(k_wind_pulse_width, k_wind_pulse_debounce, k_wind_pulse_low_level, k_wind_pulse_noise_factor, []() {});
      wind_speed_task_ = new adctask(sample_filter, wind_speed_pin);
      break;
    }

    // The falling edge method requires a falling edge task and sample counter.
    case davis6410method::falling_edge: {
      wind_speed_task_ = new fallingedgetask(wind_pulse_counter_, wind_speed_pin);
      break;
    }
  }
}

void davis6410::initialise()
{
  state_ = davis6410state::idle;
  initialised_ = true;
}

bool davis6410::start_sample(windsamplefn fn, void *context)
{
  // Must be initialised and idle.
  if (!initialised_ || state_ != davis6410state::idle) return false;

  sample_fn_ = fn;
  context_ = context;

  state_ = davis6410state::new_sample;

  return true;
}

// Abort the current sample if there is one in progress.
void davis6410::abort_sample()
{
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

void davis6410::service()
{
  switch (state_) {
    case davis6410state::idle: {
      break;
    }

    case davis6410state::new_sample: {
      // Start a new sample off.
      wind_pulse_counter_->clear();
      state_ = davis6410state::sampling_speed;

      break;
    }

    case davis6410state::sampling_speed: {
      // Check if the sample frame has finished.
      if (wind_pulse_counter_->finished()) {
        sample_pulse_count_ = wind_pulse_counter_->value();

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

float davis6410::get_wind_mph() const { return sample_pulse_count_ * 2.25f * 1000.f / static_cast<float>(sample_period_); }

int davis6410::get_wind_direction() const { return (sample_direction_ + 31) >> 6; }

uint8_t davis6410::get_pulses() const { return sample_pulse_count_; }
