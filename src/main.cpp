#include <Arduino.h>

#include "davis6410.h"


// Create the wind meter for reading the 6410 using the default pins and sample time.
davis6410 wind_meter;

void setup() {
  
  Serial.begin(9600);

  // Teh 6410 interface must be initialised before use.
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
