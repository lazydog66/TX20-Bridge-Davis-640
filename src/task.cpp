
#include <Arduino.h>

#include "task.h"

task::~task()
{
  if (filter_) delete filter_;
}

void task::start()
{
  if (!started_) {
    start_task();
    started_ = true;
  }
}

void task::stop()
{
  if (started_) {
    stop_task();
    started_ = false;
  }
}

void task::service(uint8_t sample_value)
{
  if (filter_) filter_->process_sample(sample_value);
}

void task::set_filter(class filter *sample_filter)
{
  // Delete the task's current filter if it has one.
  if (filter_) delete filter_;

  // Set the new one -pointer is owned.
  filter_ = sample_filter;
}
