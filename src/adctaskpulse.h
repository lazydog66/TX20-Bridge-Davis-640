#include "adctask.h"
#include "movingaverage.h"

class adctaskpulse : public adctask
{
  public:
  adctaskpulse(uint16_t period, uint8_t pulse_width, uint8_t debounce_width, uint8_t low_level, uint8_t pin);
  
  // Start the task.
  // Starting the task starts a new average.
  void start() override;

  // Stop the task.
  void stop() override;

  // Return true if the average has been taken.
  bool finished() const;
  
  // Return the sample average.
  // Should only be called when finished.
  uint8_t value() const { return pulse_count_; }
  uint8_t sum() const {return moving_average_.sum();}

  // Accept and proces a new sample.
  // Called from the background adc isr.
  void service(uint8_t sample_value) override;

 private:
  // The counting period in milliseconds.
  unsigned long period_ = 0;

  // This is the minimum width for a pulse to be detected, measured in samples count.
  uint8_t pulse_width_ = 0;

  // Count the number of samples since the last pulse.
  // This is used to debounce the pulses.
  uint16_t debounce_width_ = 0;

  // This is the threshold value for a logic low, ie 0 when <= low_level_.
  uint8_t low_level_ = 0;

  // This is the moving average calculator.
  movingaverage moving_average_;

  // This will be set to true when the counting interval is over.
  volatile mutable bool finished_ = true;

  // The pUlse counter.
  volatile uint8_t pulse_count_;

  // The debounce counter.
  uint8_t debounce_count_ = 0;

  // The start time for the current session..
  volatile unsigned long start_t_ = 0;
};
