#include "adctask.h"

class adctaskaverage : public adctask
{
 public:
  adctaskaverage(uint8_t pin, uint8_t n = 1) : adctask(pin), n_{n} {}

  // Start the task.
  // Starting the task starts a new average.
  void start() override
  {
    sum_ = 0;
    count_ = 0;
    finished_ = false;

    // Start this adc task.
    start_task(this, adc_pin_);
  }

  // Stop the task.
  void stop() override
  {
    finished_ = true;

    // Stop this adc task.
    stop_task(this);
  }

  // Return true if the average has been taken.
  bool finished() const { return finished_; }

  // Return the sample average.
  // Should only be called when finished.
  uint8_t value() const { return sum_ / count_; }

  // Accept and proces a new sample.
  // Called from the background adc isr.
  void service(uint8_t sample) override
  {
    if (finished_) return;

    // Update the running sum and check if finished.
    sum_ += sample;
    finished_ = ++count_ == n_;
  }

 private:
  // The number of samples to average.
  uint8_t n_ = 1;

  // The current number of samples taken.
  volatile uint8_t count_ = 0;

  // The running sample sum.
  volatile uint16_t sum_ = 0;

  // Will be true when the task is finished.
  volatile bool finished_ = false;
};
