#include <Arduino.h>

#include "filter.h"

filter::~filter()
{
  if (forward_filter_) delete forward_filter_;
}

void filter::set_forward_filter(filter* forward_filter)
{
  if (forward_filter_) delete forward_filter_;

  forward_filter_ = forward_filter;
}

  void filter::forward_sample(uint8_t value)
  {
    if (forward_filter_) forward_filter_->process_sample(value);
  }
