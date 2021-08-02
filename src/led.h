#pragma once

#include <Arduino.h>

// These are the states the led can be in,
//    off - continuously off
//    on - continusously on
//    flash - flash on for a period then continuously off
enum class ledmode : uint8_t { off, on, flash };

class led
{

public:

  // Attach an led to a particular pin, and set initially on or off.
  led(uint8_t pin, bool state = false);

  // The destructor ensures the led is off and removed from the service list.
  ~led();

  // Turn the led continuously on.
  void on();

  // Turn the led continuously off.
  void off();

  // Flash the led for a period of time which is specified in ms.
  void flash(uint16_t period_ms = 250);

  // Service the led, call peridodically and ideally as fast as possible.
  void service();

private:

  // Turn the led on or off.
  void set(bool state);

  // The pin the led is attached to.
  uint8_t led_pin_ ;

  // The currently selected mode for this led.
  ledmode mode_;

  // For a flashing led,this is how long it stays lit for.
  uint16_t led_period_;

  // If flashing, the start time of the flash in ms.
  uint16_t led_start_t_;
};
