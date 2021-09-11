
#include "tx20emulator.h"

#include "Arduino.h"
#include "windmeterintf.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

const char* winddrn_to_string(int drn) {
  // This is a look up table for converting 4 bit TX20 wind direction values to
  // named compass directions.
  static const char* directions[] = { "N",  "NNE", "NE", "ENE", "E",  "ESE",
                                     "SE", "SEE", "S",  "SSW", "SW", "WSW",
                                     "W",  "WNW", "NW", "NNW" };

  return drn >= 0 && drn <= 15 ? directions[drn] : "unknown";
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// Conversion factor from seconds to microsecondss.
constexpr float k_microseconds = 1e6;

using duration = unsigned long;

// This is the minimum time after Dtr is taken low for the emulator to 'wake' up
// and start transmitting data frames.
constexpr duration k_dtr_wakeup_interval = 1.0 * k_microseconds;

// Expected duration between sequential frames.
constexpr duration k_frame_interval = 2.5 * k_microseconds;

// Minimum time between successive frames.
constexpr duration k_frame_min_interval =
k_frame_interval - 0.5 * k_microseconds;

// The number of bits in a frame.
constexpr int k_frame_bit_count = 41;

// The length of a data bit in microseconds.
constexpr duration k_frame_bit_length = 0.002 * k_microseconds;
// constexpr duration k_frame_bit_length = 0.00122 * k_microseconds;

// Frame duration in microseconds.
constexpr duration k_frame_duration = k_frame_bit_count * k_frame_bit_length;

// ------------------------------------------------------------------------------------------------
// Constructor.
// ------------------------------------------------------------------------------------------------
tx20emulator::tx20emulator(int dtr_pin, int txd_pin)
  : dtr_pin_{ dtr_pin }, txd_pin_{ txd_pin } {
}

// ------------------------------------------------------------------------------------------------
// Initialise the emulator.
// ------------------------------------------------------------------------------------------------
void tx20emulator::initialise(windmeterintf* wind_meter, tx20eventhandler event_fn) {

  // The led is used to show the state of dtr.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // dtr needs to be pulled low for the tx20 to be active.
  // dtr is sinked low to enable the tx20.
  pinMode(dtr_pin_, INPUT_PULLUP);

  // The frame bits are transmitted on txd.
  pinMode(txd_pin_, OUTPUT);
  digitalWrite(txd_pin_, HIGH);

  wind_meter_ = wind_meter;
  event_fn_ = event_fn;

  initialised_ = true;

  set_state(tx20state::disabled);
}

// ------------------------------------------------------------------------------------------------
// Service the tx20 emulator.
//
// The emulator is implemented as a state machine.
// When the tx20 is in the inactive state, it looks to see if Dtr is low. If it
// is pulled low then it enters the wake up phase. The wake up phase is then
// followed by the sampling and sending phases. If dtr is still low the at the
// end of the sending phase, the emulator loops back to the wake up phase and so
// on. If Dtr goes high, the emulator enters the inactive phase.
//
// The built in led is lit while the tx20 emulator is sampling and sending.
// ------------------------------------------------------------------------------------------------
void tx20emulator::service() {
  if (!initialised_) return;

  switch (state_) {
    case tx20state::nothing: {
        // This state should never be enetered here.
        break;
      }

    case tx20state::disabled: {
        // Check if Dtr has gone low.
        // If it has then the tx20 enters the enabled state and starts sampling.
        if (!read_dtr()) {
          // Start a new wind sample and when complete set the state to sending.
          set_state(tx20state::start_sample);
        }

        break;
      }

    case tx20state::start_sample: {

        // Raise the start sample event.
        raise_event(tx20event::start_sample);

        set_state(tx20state::sampling);

        // Start a new wind sample and when complete set the state to sending.
        wind_meter_->start_sample(
          [](void* context) {
            tx20emulator* self = static_cast<tx20emulator*>(context);
            self->set_state(tx20state::sending);
          },
          static_cast<void*>(this));

        break;
      }

    case tx20state::sampling: {
        // While sampling, monitor the dtr line.
        // If it goes high then abort the sample and enter the disabled state.
        if (read_dtr()) {
          wind_meter_->abort_sample();
          set_state(tx20state::disabled);
          raise_event(tx20event::abort_sample);
        }

        break;
      }

    case tx20state::sending: {

        // The sending state is atomic, ie it starts and finishes in the same service call.

        // Raise the start event.
        raise_event(tx20event::start_data_frame);

        // Send the tx20 data frame and then continue.
        float mph = wind_meter_->get_wind_mph();
        int direction = wind_meter_->get_wind_direction();

        write_frame(mph, direction);

        // Raise the end event.
        raise_event(tx20event::end_data_frame);

        // Check if dtr is still low, and if not disable the tx20.
        // Otherwise continue with another sample.
        if (read_dtr())
          set_state(tx20state::disabled);
        else
          set_state(tx20state::start_sample);

        // Raise the sample end event.
        raise_event(tx20event::end_sample);

        break;
      }
  }
}

// ------------------------------------------------------------------------------------------------
// Set the internal state of the tx20 emulator.
// This sets the state but also sets the level of Txd and the built in led.
// ------------------------------------------------------------------------------------------------
void tx20emulator::set_state(tx20state state) {
  // Must be intialised and be a new state.
  if (!initialised_ || state == state_) return;

  switch (state) {
    // This state should never be set.
    case tx20state::nothing:
      break;

    case tx20state::disabled: {
        // Txd is set high when the tx20 is disabled.
        digitalWrite(txd_pin_, HIGH);
        break;
      }

    case tx20state::start_sample: {
        // Txd is set low.
        digitalWrite(txd_pin_, LOW);
        break;
      }

    case tx20state::sampling: {
        // Txd is set low while sampling.
        digitalWrite(txd_pin_, LOW);
        break;
      }

    case tx20state::sending: {
        // Txd is set low at the start of the frame..
        digitalWrite(txd_pin_, LOW);
      }
  }

  state_ = state;
}

// ------------------------------------------------------------------------------------------------
  // Send an event only if there is an event listener attached.
// ------------------------------------------------------------------------------------------------
void tx20emulator::raise_event(tx20event event) const {
  if (event_fn_) event_fn_(event);
}

// ------------------------------------------------------------------------------------------------
// Write a data frame to txd.
//
// Given a wind direction and speed, a tx20 frame is written to the txd pin.
// The frame consists of 41 bits which include  crc check on the data.
// The wind speed uses units of 0.1 metres per second.
// ------------------------------------------------------------------------------------------------
void tx20emulator::write_frame(float mph, int direction) const {

  // Need to convert the wind speed from mph to units of  0.1 meters per second.
  int units = round(mph * 1.609344 * 1000.f * 10.f / 3600.f);

  // The first half of the frame uses normal bits and the second uses inverted
  // bits.
  int windspeed1 = units;
  int winddrn1 = direction;
  int winddrn2 = ~winddrn1;
  int windspeed2 = ~windspeed1;

  // Calculate the checksum.
  int checksum = winddrn1 + (windspeed1 & 0xf) + ((windspeed1 & 0xf0) >> 4) +
    ((windspeed1 & 0xf00) >> 8);

  checksum &= 0xf;

  // Write the header which is 00100.
  // Optionally, insert a bit errorin the header.
  write_txd(LOW);
  write_txd(LOW);
  write_txd(HIGH);
  write_txd(LOW);
  write_txd(LOW);

  // Write the 4 bit wind direction.
  int data = winddrn1;
  for (int i = 0; i < 4; ++i) {
    write_txd(data & 0x01);
    data = data >> 1;
  }

  // Write the 12 bit wind speed.
  data = windspeed1;
  for (int i = 0; i < 12; ++i) {
    write_txd(data & 0x01);
    data = data >> 1;
  }

  for (int i = 0; i < 4; ++i) {
    write_txd(checksum & 0x01);
    checksum = checksum >> 1;
  }

  // Write the 4 bit wind direction.
  data = winddrn2;
  for (int i = 0; i < 4; ++i) {
    write_txd(data & 0x01);
    data = data >> 1;
  }

  // Write the 12 bit wind speed.
  data = windspeed2;
  for (int i = 0; i < 12; ++i) {
    write_txd(data & 0x01);
    data = data >> 1;
  }

  // That's the end of the data frame.
  // What we do here is write a few more end bits to give whatever is reading Txd some
  // time to decide what to do with Dtr. If Dtr is left low then another capture phase
  // will be entered, but on the other hand if Dtr is allowed to float high then a
  // the sampling is disabled and Txd will go high.
  for (int i = 0; i < 10; ++i) write_txd(LOW);
}

// ------------------------------------------------------------------------------------------------
// Read the level on the Dtr pin.
// A low enables the tx20 and a float/high disables it.
// ------------------------------------------------------------------------------------------------
bool tx20emulator::read_dtr() const { return digitalRead(dtr_pin_); }

// ------------------------------------------------------------------------------------------------
// Write a data bit to the TxD line.
// The data pulse lasts k_frame_bit_length microseconds... approx 1.22 ms.
// ------------------------------------------------------------------------------------------------
void tx20emulator::write_txd(bool data) const {
  const duration t = micros();
  digitalWrite(txd_pin_, !data);

  while (micros() - t < k_frame_bit_length)
    ;
}
