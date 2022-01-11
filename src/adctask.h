#pragma once

#include <Arduino.h>

// This is the maximum sampling rate.
constexpr uint32_t k_max_sample_rate = 31250;

//
// Initialise the adc tasks.
//
// The adc tasks run in the background on timer 1 interrupts.
// Sampling frequency is determined by timer 1.
//
void init_adc_tasks();

//
// Set the current adc task.
//
// The background adc task isr calls the task whenever it has a new sample.
//
void set_adc_task(class adctask* task);

//
// Clear the current adc task.
//
// This will clear the task only if it is the current task.
//
void clear_adc_task(class adctask* task);

class adctask
{

public:
  virtual ~adctask();

  // Return the sampling frequency used for the current sample set.
  // At the moment, the sampling rate is fixed.
  uint32_t sample_rate() const { return k_max_sample_rate; }

  // Start a new sample set.
  // Pass the adc channel for the convertions.
 // virtual void start(uint8_t pin);

  // Service the task.
  // Called by the background adc isr when each sample has been acquired.
  virtual void service(uint8_t sample_value) = 0;

protected:
  adctask(uint8_t adc_pin);

  // Start a new task, eg a sample average task.
  // Hooks the task into the background timer service.
  // Derived classes should extend start() by doing whatever initialisation they need.
  virtual void start();

private:

  // Which adc channel to sample on.
  uint8_t adc_pin_ = 0;
};
