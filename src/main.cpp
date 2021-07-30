#include <Arduino.h>

#include "davis6410.h"

// using microseconds_t = unsigned long;
// using milliseconds_t = unsigned long;

// // Debounce period for the wind speed pulses.
// // At 200 mph we have 1 pulse per 11.26 ms. With a realistic max wind speed of
// // 130 mph we can expect a pulse every 17.32 ms. Therefore a debounce period of
// // less than 17 ms should suffice.
// constexpr milliseconds_t k_wind_pulse_debounce = 10;

// // The counter for the wind pulses.
// // The anenometer spins at 1600 rev/hrs at 1 mph, or 0.444r pulses per second
// // per 1 mph. For a max speed of 200 mph this works out 88.888r pulses per
// // second or 222.222r pulses in a 2.5 second sampling period. An 8 bit counter
// // should be more than sufficient.
// volatile uint8_t wind_speed_pulse_counter = 0;

// // At 200 mph we have 1 pulse per 11.26 ms. With a realistic max wind speed of
// // 130 mph we can expect a pulse every 17.32 ms. Therefore a debounce period of
// // less than 17 ms should suffice.
// volatile milliseconds_t debounce_start_t = 0;

// float WindSpeed;  // speed miles per hour

// // --------------------------------------------------------------------------------------------------------------------
// // The isr for servicing the wind speed reading.
// // The variable debounce_start_t should be cleared before the first interrupt of
// // a sample period.
// // --------------------------------------------------------------------------------------------------------------------
// void isr_rotation() {
//   milliseconds_t now = millis();
//   if (now - debounce_start_t >= k_wind_pulse_debounce) {
//     ++wind_speed_pulse_counter;
//     debounce_start_t = now;
//   }
// }

// // --------------------------------------------------------------------------------------------------------------------
// // Convert wind speed pulse count to kph.
// // This is done as a function so that in future, it will be easy to add a look
// // up table for calibrating the anenometer if we think it needs one. For now,
// // though, we assume that the formula V=P(2.25/T) given in the spec is correct.
// // --------------------------------------------------------------------------------------------------------------------
// float calculate_wind_mph(uint8_t pulses) {
//   float v_mph =
//       pulses * 2.25f * 1000.f / static_cast<float>(k_wind_speed_sample_t);

//   return v_mph;
// }

// // --------------------------------------------------------------------------------------------------------------------
// // Convert wind speed pulse count to kph.
// // --------------------------------------------------------------------------------------------------------------------
// float calculate_wind_kph(uint8_t pulses) {
//   float v_mph = calculate_wind_mph(pulses);
//   float v_kph = v_mph * 1.60934f;

//   return v_kph;
// }

// // --------------------------------------------------------------------------------------------------------------------
// // Sample the wind speed.
// // Returns the speed in mpp.
// // --------------------------------------------------------------------------------------------------------------------
// int sample_wind() {
//   // We're only interested in the speed to the nearest mph so using delay()
//   // should be accurate enough.
//   wind_speed_pulse_counter = 0;
//   delay(k_wind_speed_sample_t);

//   auto pulses = wind_speed_pulse_counter;

//   return pulses;
// }

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

// Create the wind meter for reading the 6410 using the default pins and sample time.
davis6410 wind_meter;

void setup() {
  
  Serial.begin(9600);

  wind_meter.initialise();
}

void loop() {
  // Read the wind speed.
  // We're only interested in the speed to the nearest mph so this way should be
  // accurate enough.
  float mph = wind_meter.sample_wind_mph();
  int direction = wind_meter.sample_wind_direction();

  Serial.print("wind speed: mph=");
  Serial.print(mph);
  Serial.print(", direction=");
  Serial.println(direction);
}
