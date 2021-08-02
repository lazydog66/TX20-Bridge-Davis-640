#include <Arduino.h>

#include "davis6410.h"
#include "tx20emulator.h"
#include "led.h"

// Create the wind meter for reading the 6410 using the default pins and sample time.
davis6410 wind_meter;

// Create the tx20 emulator for sending tx20 formatted wind data.
tx20emulator tx20_eumlator;

// Use an led to signal when samples are taken.
led tx20_led(LED_BUILTIN);

// This is the event handler for the tx20 emulator events.
// What we do is flash the led when a wind sample has been taken and the data
// is being sent out on the Txd line.
void tx20_event_handler(tx20event event) {

  switch (event) {

    case tx20event::start_data_frame: {
        // Flash the led when data is being sent out on Txd.
        tx20_led.flash(250);
        break;
      }

    case tx20event::end_sample: {
        // At this point, the wind has been sampled and the data sent on Txd.
        // As an example, the wind sample is logged to the console.
        float mph = wind_meter.get_wind_mph();
        int direction = wind_meter.get_wind_direction();

        Serial.print(String(F("wind speed: mph=")) + String(mph));
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

void setup() {

  Serial.begin(115200);

  Serial.println(F(""));
  Serial.println(F("Davis 6410 ==> TX20 Bridge v1.0.1"));
  Serial.println(F(""));

  // The 6410 interface  and tx20 emulator must be initialised before use.
  wind_meter.initialise();
  tx20_eumlator.initialise(&wind_meter, tx20_event_handler);
}

void loop() {
  // Service the 6410 interface and tx20 emulator.
  // This needs to be done periodically and as fast as possible.
  wind_meter.service();
  tx20_eumlator.service();
  tx20_led.service();
}
