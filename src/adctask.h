#pragma once

#include <Arduino.h>

#include "task.h"

// This is the maximum sampling rate.
constexpr uint32_t k_adc_sample_rate = 31250;

// void init_adc_tasks();

// //
// // Set the current adc task.
// //
// // The background adc task isr calls the task whenever it has a new sample.
// //
// void set_adc_task(class adctask *task);

// //
// // Clear the current adc task.
// //
// // This will clear the task only if it is the current task.
// //
// void clear_adc_task(class adctask *task);

//
// This is the adc task.
//
// The adc runs continuously in the background. The task hooks into the
// adc loop and acquires the samples as they are generated.
class adctask : public task
{

 public:
  // Initialise the background adc tasks.
  //
  // The adc tasks run in the background on timer 1 interrupts.
  // Sampling frequency is determined by timer 1.
  //
  // This should be called once early on.
  //static void initialise();

  // Construct the adc task with a particular filter.
  // Note, the adc task gets to own the filter pointer.
  adctask(class filter *sample_filter, uint8_t adc_pin);
  ~adctask();

  // Return the sampling frequency used for the current sample set.
  // At the moment, the sampling rate is fixed.
  uint32_t sample_rate() const { return k_adc_sample_rate; }

  // Start a new sample set.
  // Pass the adc channel for the convertions.
  // virtual void start(uint8_t pin);

  // Service the task.
  // Called by the background adc isr when each sample has been acquired.
  void service(uint8_t sample_value) override;

 private:

  // Start the adc task
  void start_task() override;

  // Stop the adc task.
  void stop_task() override;

  // The filter currently attached to the adc task.
  class filter *filter_ = nullptr;

  // Which adc channel to sample on.
  uint8_t adc_pin_ = 0;

  // Tis is the number of samples to ignore.
  uint8_t ignore_count_ = 0;
};
