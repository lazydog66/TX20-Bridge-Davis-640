//
// This is a class for generating pulses on a pin.
//
// It's used for testing the pulse detection on the anemometer speed line.
#pragma once

class pulsegenerator
{
 public:
 // Construct the pulse generator with a particular period and width for the
 // pulses, and which ppin to output the pulses on,
  pulsegenerator(uint16_t period, uint16_t width, uint8_t pin);

  void service();

 private:
  uint16_t period_ = 0;
  uint16_t width_ = 0;
  uint8_t pin_;
};
