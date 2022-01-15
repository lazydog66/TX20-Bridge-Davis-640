#include <math.h>

#include "fallingedgetask.h"
#include "filter.h"

using microseconds_t = unsigned long;
using milliseconds_t = unsigned long;

// The counter for the wind pulses.
// The anenometer spins at 1600 rev/hrs at 1 mph, or 0.444r pulses per second
// per 1 mph. This means an 8 bit counter should easily suffice for our needs.
// Using an 8 bit counter has the advantage that we dont need to disable interrutps
// when reading or clearing the counter.
static volatile uint8_t wind_speed_pulse_counter = 0;

// This variable is needed to debounce the reed switch.
static volatile milliseconds_t debounce_start_t = 0;

static fallingedgetask* current_task = nullptr;

static bool initialised = false;

// The isr for servicing falling edge interrupts.
static void isr_6410()
{
  if (current_task) current_task->service(0);
}

// Enable falling edge interrupts on the specified pin.
// It's up to the caller to ensure the pin is valid.
static void initialise(uint8_t pin)
{
  // Initialise the falling edge interrupts if need be.
  if (!initialised) {

    cli();

    current_task = nullptr;

    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), isr_6410, FALLING);

    initialised = true;

    sei();
  }
}

fallingedgetask::fallingedgetask(class filter* sample_filter, uint8_t pin) {}

void fallingedgetask::start_task()
{
  // Ensure the falling edge interrupts have been initialised.
  initialise(pin_);

  cli();
  current_task = this;
  sei();
}

void fallingedgetask::stop_task()
{
  cli();
  current_task = nullptr;
  sei();
}

void fallingedgetask::service(uint8_t sample_value)
{
  // Use 255 for the falling edge sample value.
  if (filter_) filter_->process_sample(255);
}