
#include <Arduino.h>

#include "task.h"

task::task(filter* sample_filter)
: filter_{sample_filter}
{}

task::~task()
{
  if (filter_) delete filter_;
}

void task::start()
{
  if (!started_) {
    cli();
    start_task();
    sei();
  }
}

void task::stop()
{
  if (started_) {
    cli();
    stop_task();
    sei();
  }
}

void task::service(uint8_t sample_value)
{
  Serial.write('.');
  if (filter_) filter_->process_sample(sample_value);
}

void task::set_filter(class filter *sample_filter)
{
  // Delete the task's current filter if it has one.
  if (filter_) delete filter_;

  // Set the new one -pointer is owned.
  filter_ = sample_filter;
}
