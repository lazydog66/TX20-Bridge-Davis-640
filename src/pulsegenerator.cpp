#include <Arduino.h>

#include "pulsegenerator.h"

pulsegenerator::pulsegenerator(uint16_t period, uint16_t width, uint8_t pin) : period_{period}, width_{width}, pin_{pin} { pinMode(pin, OUTPUT); }

void pulsegenerator::service()
{
  auto t = millis() % period_;
  bool level = t > width_ ? 1 : 0;

  // Use inverted logic on the output.
  digitalWrite(pin_, level ? HIGH : LOW);
}
