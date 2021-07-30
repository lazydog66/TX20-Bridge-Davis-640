#include <Arduino.h>
#include <math.h>

#include "davis6410.h"


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
davis6410::davis6410(int wind_speed_pin, int wind_vane_pin, unsigned long sample_period)
  : wind_speed_pin_{ wind_vane_pin }, wind_vane_pin_{ wind_vane_pin }, sample_period_{ sample_period }
{
}

// --------------------------------------------------------------------------------------------------------------------
// The isr for servicing the wind speed reading.
// The variable debounce_start_t should be cleared before the first interrupt of a sample period.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::initialise()
{
  pinMode(wind_speed_pin_, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(wind_speed_pin_), isr_6410, FALLING);

  initialised_ = true;
}

// --------------------------------------------------------------------------------------------------------------------
// Start a ne sample.
// The callback will be called when the sample is ready.
// --------------------------------------------------------------------------------------------------------------------
void davis6410::start_sample(windsamplefn fn)
{
  sample_fn_ = fn;

  state_ = new_sample;
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
        // Start a new sample off, interrupts enabled.
        wind_speed_pulse_counter = 0;
        sample_start_time_ = millis();
        sei();

        break;
      }

    case sampling_speed:
      // Check if the sample frame has finished.
      if (millis() - sample_start_time_ >= sample_period_) {
        // Stop interrupts and then read the wind speed in mph.
        cli();
        wind_speed_ = calculate_wind_mph(wind_speed_pulse_counter);

        // Sample the wind direction.
        state_ = sampling_direction;
      }

      break;

    case sampling_direction: {
        // Read the wind direction directly.
        int drn = analogRead(wind_vane_pin_);
        Serial.println(drn);
        wind_direction_ = (drn + 31) >> 6;

        state_ = send_frame;

        break;
      }

    case send_frame: {
        // Let the client knowthe sampled wind speed and direction.
        sample_fn_(wind_speed_, wind_direction_);

        state_ = idle;

        break;
      }
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Sample the wind speed.
// Returns the speed in mpp.
// --------------------------------------------------------------------------------------------------------------------
// float davis6410::sample_wind_mph() const {
//   // Must be intialsied first.
//   if (!initialised_) return 0;

//   // Start a new sample frame.
//   wind_speed_pulse_counter = 0;

//   // We're only interested in the speed to the nearest mph so using delay()
//   // should be accurate enough.
//   sei();
//   delay(k_wind_speed_sample_t);
//   cli();

//   return calculate_wind_mph(wind_speed_pulse_counter);
// }

// --------------------------------------------------------------------------------------------------------------------
// Sample the wind direction.
// Returns the direction using 16 directions, 0 is North, 4 is East etc.
// --------------------------------------------------------------------------------------------------------------------
// int davis6410::sample_wind_direction() const
// {
//   int drn = analogRead(wind_vane_pin_);
//   Serial.println(drn);
//   drn = (drn + 31) >> 6;

//   return drn;
// }

// --------------------------------------------------------------------------------------------------------------------
// Convert wind speed pulse count to kph.
// This is done as a function so that in future, it will be easy to add a look
// up table for calibrating the anenometer if we think it needs one. For now,
// though, we assume that the formula V=P(2.25/T) given in the spec is correct.
// --------------------------------------------------------------------------------------------------------------------
float davis6410::calculate_wind_mph(uint8_t pulses) const {
  float v_mph =
    pulses * 2.25f * 1000.f / static_cast<float>(k_wind_speed_sample_t);

  return v_mph;
}
