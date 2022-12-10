
#include <Arduino.h>
#include <avr/wdt.h>

#include "davis6410.h"
#include "tx20emulator.h"
#include "led.h"
#include "adctask.h"
#include "pulsegenerator.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// Some settings to help debugging.
// If these are set, then the wind speed and direction an be tested by connecting the built in
// led pin directly to A2, and a potentiometer can be attached to PD2. The output should then
// display wind readings.
constexpr bool k_debug_mode = false;
constexpr uint8_t k_debug_wind_speed_mph = 10;
constexpr uint8_t k_debug_pulse_width = 2;

// The pin the front panel led is attached to.
// The led is flashed to show when a wind sample has been taken.
constexpr int k_front_panel_ped_pin = 9;

// The front panel led is flashed for this number of milliseconds when a sample has been taken.
constexpr uint16_t k_led_sample_flash_ms = 20;

// The wind sensor pin for the Davis 6410 has to be one of the adc channel
constexpr int k_wind_sensor_pin_adc = 2;

// The wind direction is measured by sampling the wind vane potentiometer in the 6410.
constexpr int k_wind_direction_pin = 0;

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
davis6410* wind_meter;

// Create the tx20 emulator for sending tx20 formatted wind data.
tx20emulator* tx20_emulator;

// Create the controller for the front panel led.
led panel_led(k_front_panel_ped_pin);

// The pulse generator is used for testing the davis640 and supporting classes.
pulsegenerator pulse_generator(2250 / k_debug_wind_speed_mph, k_debug_pulse_width, LED_BUILTIN);

//
// ------------------------------------------------------------------------------------------------
// This is the event handler for the tx20 emulator events.
// What we do is flash the led when a wind sample has been taken and the data
// is being sent out on the Txd line.
// ------------------------------------------------------------------------------------------------
void tx20_event_handler(tx20event event)
{

  switch (event) {

    case tx20event::start_data_frame: {
      // Flash the led when data is being sent out on Txd.
      panel_led.flash(k_led_sample_flash_ms);
      break;
    }

    case tx20event::end_sample: {
      // At this point, the wind has been sampled and the data sent on Txd.
      // As an example, the wind sample is logged to the console.
      uint8_t pulses = wind_meter->get_pulses();
      float mph = wind_meter->get_wind_mph();
      int direction = wind_meter->get_wind_direction();

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
void setup()
{

  Serial.begin(115200);

  delay(500);

  Serial.println(F(""));
  Serial.println(F("Davis 6410 ==> TX20 Bridge v1.0.2"));
  Serial.println(F(""));
  Serial.println(String(F("speed sample T is ")) + String(k_wind_speed_sample_t) + F(" ms"));
  Serial.println(String(F("speed signal sample frequency ")) + String(k_adc_sample_rate) + F(" Hz"));
  Serial.println(String(F("pulse min width ")) + String(k_wind_pulse_width) + F(" samples"));
  Serial.println(String(F("debounce set to ")) + String(k_wind_pulse_debounce) + F(" samples"));
  Serial.println(F(""));

  // Creat the interface to the Davis 6410.
  // The interface can use either the falling edge interrupts on pin 2 or 3, or the adc.
  wind_meter = new davis6410(k_wind_sensor_pin_adc, k_wind_direction_pin);

  // Create the tx20 emulator for sending tx20 formatted wind data.
  tx20_emulator = new tx20emulator(k_dtr_pin, k_txd_pin);

  // The 6410 interface  and tx20 emulator must be initialised before use.
  wind_meter->initialise();
  tx20_emulator->initialise(wind_meter, tx20_event_handler);

  if (k_debug_mode) {
    // Output Vcc to PD2 so that a potentiometer can emulate the wind direction.
    pinMode(PD2, OUTPUT);
    digitalWrite(PD2, HIGH);
  }

  wdt_enable(WDTO_4S);
}

// ------------------------------------------------------------------------------------------------
// The main loop simply services the  6410 interface, the tx20 emulator and the led.
// These need to be done periodically and as often as possible.
// ------------------------------------------------------------------------------------------------
void loop()
{
  // Service the 6410 interface and tx20 emulator.
  wind_meter->service();
  tx20_emulator->service();

  panel_led.service();

  // In debug mode, oulses are output on the built in led pin.
  // The pin can be attached directly to the signal input adc pin to emulate the Davis 6410.
  if (k_debug_mode) pulse_generator.service();

  wdt_reset();
}
