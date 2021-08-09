// ------------------------------------------------------------------------------------------------
// This is a simple class for controlling an led on one of the digital io pins.
// An led can be turned on, off or flahsed. After creating an led instance, call
// service() from the main loop. This will ensure that flash() works smoothly.
// ------------------------------------------------------------------------------------------------

#include "led.h"

// ------------------------------------------------------------------------------------------------
// Constructor sets the io poin for the led.
// ------------------------------------------------------------------------------------------------
led::led(uint8_t pin, bool state)
  :led_pin_{pin}, mode_{state ? ledmode::on : ledmode::off}
{
  pinMode(pin, OUTPUT);

  if (state) on();
  else off();
}

// ------------------------------------------------------------------------------------------------
// Destructor ensures the led is turned off.
// ------------------------------------------------------------------------------------------------
led::~led()
{
  set(false);
}

// ------------------------------------------------------------------------------------------------
// Turn the led continuously on.
// ------------------------------------------------------------------------------------------------
void led::on()
{
  set(true);
  mode_ = ledmode::on;
}

// ------------------------------------------------------------------------------------------------
// Turn the led continuously off.
// ------------------------------------------------------------------------------------------------
void led::off()
{
  set(false);
  mode_ = ledmode::off;
}

// ------------------------------------------------------------------------------------------------
// Flash the led on.
// It makes no difference if the led is already on.
// ------------------------------------------------------------------------------------------------
void led::flash(uint16_t period_ms)
{
  set(true);

  led_start_t_ = millis() & 0xffff;
  led_period_ = period_ms;

  mode_ = ledmode::flash;
}

// ------------------------------------------------------------------------------------------------
// Service the led.
// The on and off states don't need anything doing, but the flash does need
// checking to see if the led needs turning off.
// ------------------------------------------------------------------------------------------------
void led::service()
{
  switch (mode_)
  {
    case ledmode::off:
    case ledmode::on:
      // These states don't need anything doing.
      break;

    case ledmode::flash:
      {
        if (millis() - led_start_t_ > led_period_)
        {
          set(false);
          mode_ = ledmode::off;
        }

        break;
      }
  }
}

// ------------------------------------------------------------------------------------------------
// Set the phsical led on or off.
// ------------------------------------------------------------------------------------------------
void led::set(bool state)
{
  digitalWrite(led_pin_, state);
}
