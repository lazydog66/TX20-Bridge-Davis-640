#include <Arduino.h>

#include "davis6410.h"
#include "tx20emulator.h"


// Create the wind meter for reading the 6410 using the default pins and sample time.
davis6410 wind_meter;

// Create the tx20 emulator for sending tx20 formatted wind data.
tx20emulator tx20_eumlator;


void log_wind_sample(float mph, int direction) {
  Serial.print(F("wind speed: mph="));
  Serial.print(mph);
  Serial.print(F(", direction="));
  Serial.println(direction);

  // Start another wind sample off.
  //wind_meter.start_sample(log_wind_sample);
}

void setup() {

  Serial.begin(115200);

  Serial.println(F(""));
  Serial.println(F("Davis 6410 ==> TX20 Bridge v1.0"));
  Serial.println(F(""));

  // The 6410 interface  and tx20 emulator must be initialised before use.
  wind_meter.initialise();
  tx20_eumlator.initialise(&wind_meter);

}

void loop() {

  // Service the 6410 interface and tx20 emulator.
  // This needs to be done periodically and as fast as possible.
  wind_meter.service();
  tx20_eumlator.service();
}
