#pragma once

#include <Arduino.h>

#include "task.h"

//
// This is the falling edge task.
//
class fallingedgetask : public task
{

 public:
  // Construct the falling edge task with a particular filter.
  fallingedgetask(class filter *sample_filter, uint8_t pin);
  ~fallingedgetask();

  // Service a falling edge interrupt.
  // Basically, it passes the event on to the attached filter.
  void service(uint8_t sample_value) override;

 private:
  // Start the falling edge, task.
  void start_task() override;

  // Stop the falling edge task.
  void stop_task() override;
  
  // The pin number to attach the interrupt to.
  uint8_t pin_;
};
