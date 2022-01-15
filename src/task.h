//
// The task class is a base class for acquiring samples.
//
// Derived class are adctask and fallingedgetask.
//
// The role of a task is to abstract away the details of acquiring samples for filters.
// The filters accept and process the samples without worrying how to actually generate the data.
//

#pragma once

#include <Arduino.h>

#include "filter.h"

class task
{

 public:
  // Deleting the task will also delete the attached filter.
  ~task();

  // Start the task
  void start();

  // Stop this task.
  void stop();

  // Service the task.
  virtual void service(uint8_t sample_value) = 0;

  // Set the filter which accepts and processes the samples.
  // Note, the task gets to own the pointer.
  void set_filter(class filter* sample_filter);

 protected:
  // The sample filter associated with the task.
  filter* filter_ = nullptr;

 private:
  // Start the low level task.
  // Derived tasks will need to start any resources they need, eg falling edge trigger.
  virtual void start_task() = 0;

  // Stop the low level task.
  // For example, stop interrupts happening on pins, stop the adc.= etc.
  // Derived classes need to implement stop_task().
  virtual void stop_task() = 0;

  // This will be true if the task has been started.
  // Derived classes should extend this method to start up whatever resources they need.
  bool started_ = false;
};
