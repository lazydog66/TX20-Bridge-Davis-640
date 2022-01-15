
#include <Arduino.h>

#include "samplecounter.h"

samplecounter::samplecounter(uint16_t period) : period_{period} {}

void samplecounter::clear()
{
  // process_sample() can be called by an isr, so we need to be careful
  // that count_ is incremented when it shouldn't be.
  cli();

  count_ = 0;
  finished_ = false;
  start_t_ = millis();

  sei();
}

bool samplecounter::finished() const
{
  if (finished_) return true;

  finished_ = millis() - start_t_ >= period_;

  return finished_;
}

void samplecounter::process_sample(uint8_t)
{
  if (finished_) return;

  finished_ = millis() - start_t_ >= period_;

  if (!finished_) ++count_;
}