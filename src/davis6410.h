// ------------------------------------------------------------------------------------------------
// Controller for the Davis 6410 wind meter.
//
// The wind meter measures wind speed and direction. The speed is measured by
// counting pulses prodcued by a reed switch in the unit. The direction is encoded by a
// 20KOhm potentiometer also in the unit.
// --------------------------------------------------------------------------------------------------------------------
#pragma once

#include <Arduino.h>

#include "adctask.h"
#include "windmeterintf.h"

// This is the number of samples to take when calcualting the wind direction.
constexpr uint8_t k_wind_direction_average_n = 10;

// This is the default duration over which the wind speed is calculated.
// The anenometer's spec says the minimum wind speed is 1 mph which is 1
// revolution per 2.25 seconds, so 2.25 seconds seems like a reasonable amount
// of time. This has the advantage that the returned wind speed will be an
// integer number of 1 mph.
constexpr unsigned long k_wind_speed_sample_t = 2250;

// Debounce period for the wind speed pulses.
// Information on the web suggests that the debounce period for a reed switch
// is around 1 ms. At 200 mph we have 1 pulse per 11.26 ms (for a 2.25 second sample
// period), hece something in the range 1 to 20 ms will do.
constexpr unsigned long k_wind_pulse_debounce = 8;

// This is the minimum width, in millieconds, of a pulse on the wind speed sensor line.
constexpr uint8_t k_wind_pulse_width = 4; // 3 * k_adc_sample_rate / 1000;

// This is the threshold value for a logic low on the wind speed sensor line.
constexpr uint8_t k_wind_pulse_low_level = 10;

// This is the threshold for detecting noisy pulses.
// A value of 0 will only detect pulses with no noise, wheras a value 0f 0.1 will allow
// up to 10% of the samples to be noise.
constexpr float k_wind_pulse_noise_factor = 0.03f;

// The wind speed signal can be sampled by either the falling edge task or the adc task.
// This enum is used to describe which method should be used.
//enum class davis6410method { adc, falling_edge };

// The state for the 6410.
//    idle - the 6410 is doing nothing
//    new_sample - a new sample has been requested
//    sampling_speed -
enum class davis6410state {
  idle,
  new_sample,
  sampling_speed,
  sampling_direction,
  send_frame,
};

class davis6410 : public windmeterintf
{
 public:
  // The Davis runs off two pins, a digital input for the wind speed pulses and
  // an analogue pin for the wind direction. The anenometer's spec says the
  // minimum wind speed is 1 mph which is 1 revolution per 2.25 seconds, so
  // 2.25 seconds for the period has the advantage that the returned pulse count
  // is the wind speed in mph. The wind speed can be sampled either using
  // falling edge interrupts or with the adc.
  davis6410(int wind_sensor_pin, int wind_direction_pin, unsigned long sample_period = 2250);

  // Initialise the hardware resources and set up the isr.
  // This must be done once before the 6410 can be used.
  void initialise();

  // Service the interface.
  void service();

  // Start a new sample.
  // The callback will be called when the sample is ready.
  // Returns true if the sample was started, false otherwise.
  bool start_sample(windsamplefn fn, void* context) override;

  // Abort the current sample if there is one in progress.
  void abort_sample() override;

  // Return the last sampled wind speed.
  // The calcualtion from pulse count to mph uses the formula V=P(2.25/T).
  // If it's found that it is not accurate enough then calibration tables could be used.
  float get_wind_mph() const override;

  // Return the last sampled wind direction.
  // Returns the direction as 0=N, E=4 etc.
  int get_wind_direction() const override;

  // Return the last sampled anenometer pulse count.
  uint8_t get_pulses() const;

  // Return the state of the Davis 6410.
  davis6410state state() const { return state_; }

 private:
  // Convert pulses to mph.
  // Note, this may in the future apply calibration data to the result.
  float calculate_wind_mph(uint8_t pulses) const;

  // A digital pin is used to counting the anenometer pulses.
  const int wind_speed_pin_;

  // The analog pin to use for reading the wind vane direction.
  const int wind_vane_pin_;

  // The duration in milliseconds of the sample period.
  unsigned long sample_period_;

  // The resources must be initialised before the 6410 can be read.
  bool initialised_ = false;

  // The state of the interface.
  davis6410state state_ = davis6410state::idle;

  // This is the pulse count for the last sample frame.
  uint8_t sample_pulse_count_;

  // This is the last analogue reading for the wind direction.
  int sample_direction_;

  // The wind sample callback function.
  windsamplefn sample_fn_ = nullptr;

  // A context that is passed to the callback function.
  void* context_ = nullptr;

  // The task for acquiring the wind direction.
  class adctaskaverage* wind_direction_task_ = nullptr;

  // The task for sampling the wind speed.
  // The task can be either a falling edge task or adc task.
  class adctaskpulse* wind_speed_task_ = nullptr;
};
