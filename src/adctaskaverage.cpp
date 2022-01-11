#include "adctaskaverage.h"

// How many samples to igore at the srar of a new average.
constexpr uint8_t k_ignore_samples_count = 8;

adctaskaverage::adctaskaverage(uint8_t adc_pin)
    : adctask{adc_pin}
{
}

void adctaskaverage::start(uint8_t n)
{
    cli();

    n_ = n;
    count_ = 0;
    sum_ = 0;
    ignore_ = k_ignore_samples_count;
    finished_ = 0;

    sei();
}

uint8_t adctaskaverage::average() const
{
    // TODO Improve by rounding.
    return sum_ / count_;
}

void adctaskaverage::service(uint8_t sample)
{
    // If enough samples have already been taken, then there is nothing to do.
    if (finished_)
        return;

    if (ignore_)
        --ignore_;
    else
    {
        // Update the running sum.
        sum_ += sample;

        // Check if finished.
        finished_ = ++count_ == n_;
    }
}