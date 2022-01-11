#include "adctask.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// Timer 1 is set to run slower than the adc. In this way, the sample
// rate is determined by the running rate of timer 1. The adc runs at ~ 38 KHz.
// A prescaler of 64 and ctc value of 7 gives a timer frequency of,
//    16000000 / (64 * 8) = 31250 Hz
// This is slower than the adc, so should be okay.
constexpr int k_timer_1_comapre_to = 7;

// This is the frequency that timer 1 is set to run at.
constexpr uint32_t k_timer_1_frequency = 31250;

// This value drives the adc at ~ 38.5 KHz, which is faster than timer1.
constexpr uint8_t k_adc_prescaler = 32;

// The current adc channel in use.
static uint8_t adc_pin = 255;

// The adc channel to use for the next convertion.
static uint8_t next_adc_pin = 0;

// The currently active adc task.
static adctask *adc_task = nullptr;

//
// Utility function to start a new adc conversion.
//
// Setting bit ADSC starts the conversion.
//
static void analog_trigger()
{
    sbi(ADCSRA, ADSC);
}

//
// Utility function to return whether the adc value is ready to be read.
//
static bool analog_ready() { return !bit_is_set(ADCSRA, ADSC); }

//
// Utility function to read the current adc value.
// The adc is set up to sample at 8 bit resolution and left align the result, ie
// ADCH contains the result.
//
static uint8_t analog_read() { return ADCH; }

//
// This is timer 1's service routine.
//
// All we do is service the adc and call whatever adc task is currently in use.
//
ISR(TIMER1_COMPA_vect)
{
    // Nothing to do if the adc hasn't finished.
    // In theory, the adc convertion should always be ready, except perhaps for
    // the first adc after the adc channel has been changed. Strictly speaking, the first
    // sample may not be entirely accurate then.
    if (!analog_ready())
        return;

    // Get the sample.
    uint8_t sample = analog_read();

    // Change the channel if need be, and then start the next adc convertion going.
    if (next_adc_pin != adc_pin)
    {
        // Set up the adc for the selected channel and left align the result.
        // Left aligning allows the 8 bit value to be read straight from ADCH.
        // Setting bit 6 selects AVCC as the voltage reference.
        // See
        // https://www.arnabkumardas.com/arduino-tutorial/adc-register-description/.
        // pin = analogPinToChannel(pin);
        ADMUX = (1 << 6) | (1 << 5) | (next_adc_pin & 0x07);

        adc_pin = next_adc_pin;
    }

    analog_trigger();

    // Pass teh sample on to the current adc task.
    if (adc_task)
        adc_task->service(sample);
}

//
// Initialise the adc.
//
// The clock prescaler determines the frequency at which the adc will run at.
// In addition, the adc is set to use 8 bit resolution and left align the
// result. Left laigning te result makes reading the adc value easier.
//
static void init_adc_clock_prescaler(uint8_t value)
{
    static uint8_t adc_prescale = 0;

    // Nothing to do if using the current prescaler value.
    if (value == adc_prescale)
        return;

    switch (value)
    {
    case 2:
    {
        sbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
    }

    case 4:
    {
        cbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
    }

    case 8:
    {
        sbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
    }

    case 16:
    {
        cbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
    }

    case 32:
    {
        sbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
    }

    case 64:
    {
        cbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
    }

    case 128:
    default:
    {
        sbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
    }
    }
}

//
// Initialise the adc task and timer.
//
// The adc is driven directly by timer 1 interrupts.
//
static void initialise_timer_and_adc()
{
    // Set up timer 1 interrupts.
    noInterrupts();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Prescaler is 64.
    OCR1A = k_timer_1_comapre_to;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);

    // Make sure digital inputs are disabled on the adc channels.
    // sbi(DIDR0, ADC0D);
    // sbi(DIDR0, ADC1D);
    // sbi(DIDR0, ADC2D);
    // sbi(DIDR0, ADC3D);
    // sbi(DIDR0, ADC4D);
    // sbi(DIDR0, ADC5D);
    // sbi(DIDR0, ADC6D);
    // sbi(DIDR0, ADC7D);

    // Speed up the adc clock divider.
    // The adc should be driven at a rate faster than the timer 1 interrupts.
    init_adc_clock_prescaler(k_adc_prescaler);

    // Initialise the adc by performing a conversion.
    // According to the spec sheet, the first adc conversion takes 25 cycles as
    // opposed to 13 for subsequent conversions. This is a bit of a problem
    // because the adc task runs at 6250 Hz which is too fast for 25 cycles. By
    // performing a conversion now we can initalise the adc and get the 25 cycle
    // start up over with.
    // analog_trigger(adc_select_pin);
    // delay(1);

    interrupts();

    // Start the adc off in the background.
    analog_trigger();
}

void set_adc_task(adctask *task)
{
    cli();
    adc_task = task;
    sei();
}

void clear_adc_task(adctask *task)
{
    if (adc_task == task)
    {
        cli();
        adc_task = nullptr;
        sei();
    }
}

adctask::adctask(uint8_t adc_pin)
    : adc_pin_{adc_pin}
{
}

adctask::~adctask()
{
    clear_adc_task(this);
}

void adctask::start_task()
{
    cli();

    // Make this the current task.
    adc_task = this;
    
    // Set up which adc channel we're sampling on.
    next_adc_pin = adc_pin_;

    // Start the sample convertions.
    analog_trigger();

    sei();
}
