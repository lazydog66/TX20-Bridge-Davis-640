
#include <Arduino.h>

#include "davis6410.h"
#include "tx20emulator.h"
#include "led.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// The pin the front panel led is attached to.
// The led is flashed to show when a wind sample has been taken.
constexpr int k_front_panel_ped_pin = 9;

// The front panel led is flashed for this number of milliseconds when a sample has been taken.
constexpr uint16_t k_led_sample_flash_ms = 333;

// The Davis 6410 interface uses two pins.
// The wind sensor pin is used to count pulses from the anenometer using interrupts. We muse us
// a pin that supports interrupts. The wind direction is measured by sampling the wind vane
// potentiometer in the 6410. An analoue pin is used to do this.
constexpr int k_wind_sensor_pin = 2;
constexpr int k_wind_direction_pin = A0;

// The TX20  emulator uses two digital pins for Dtr and Txd which are defined here.
// Dtr is an input and controls whether the TX20 should sample and send wind data.
// Txd is an output and is used to send the sampled wind speed and direction.
constexpr int k_dtr_pin = 3;
constexpr int k_txd_pin = 4;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// Create the interface for reading the 6410.
// We'll use the default sampling period which is 2250 milliseconds. This is a convenient
// duration because it means that the wind speed in mph is simply the number of pulses in the sample.
davis6410 wind_meter(k_wind_sensor_pin, k_wind_direction_pin);

// Create the tx20 emulator for sending tx20 formatted wind data.
tx20emulator tx20_emulator(k_dtr_pin, k_txd_pin);

// Create the controller for the front panel led.
led panel_led(k_front_panel_ped_pin);

// ------------------------------------------------------------------------------------------------
// This is the event handler for the tx20 emulator events.
// What we do is flash the led when a wind sample has been taken and the data
// is being sent out on the Txd line.
// ------------------------------------------------------------------------------------------------
void tx20_event_handler(tx20event event) {

  switch (event) {

    case tx20event::start_data_frame: {
        // Flash the led when data is being sent out on Txd.
        panel_led.flash(k_led_sample_flash_ms);
        break;
      }

    case tx20event::end_sample: {
        // At this point, the wind has been sampled and the data sent on Txd.
        // As an example, the wind sample is logged to the console.
        uint8_t pulses = wind_meter.get_pulses();
        float mph = wind_meter.get_wind_mph();
        int direction = wind_meter.get_wind_direction();

        Serial.print(String(F("pulses=")) + String(pulses));
        Serial.print(String(F(", mph=")) + String(mph));
        Serial.println(String(F(", direction=")) + String(direction));

        break;
      }

    // Don't bother do anything for these events.
    case tx20event::start_sample:
    case tx20event::end_data_frame:
    case tx20event::abort_sample: {
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Set up initaialse the 6410 interface and tx20 emulator.
// ------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);

  Serial.println(F(""));
  Serial.println(F("Davis 6410 ==> TX20 Bridge v1.0.1"));
  Serial.println(F(""));
  Serial.println(String(F("speed sample T is ")) + String(k_wind_speed_sample_t)+ F(" ms"));
  Serial.println(String(F("debounce set to ")) + String(k_wind_pulse_debounce)+ F(" ms"));
  Serial.println(F(""));

  // panel_led.on();
  // delay(3000);
  // panel_led.off();

  // The 6410 interface  and tx20 emulator must be initialised before use.
  wind_meter.initialise();
  tx20_emulator.initialise(&wind_meter, tx20_event_handler);
}

// ------------------------------------------------------------------------------------------------
// The main loop simply services the  6410 interface, the tx20 emulator and the led.
// These need to be done periodically and as often as possible.
// ------------------------------------------------------------------------------------------------
void loop() {
  // Service the 6410 interface and tx20 emulator.
  wind_meter.service();
  tx20_emulator.service();
  panel_led.service();
}
