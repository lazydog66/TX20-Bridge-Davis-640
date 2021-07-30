
#include "Arduino.h"

#include "tx20emulator.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

const char* winddrn_to_string(winddrn drn) {
  // This is a look up table for converting 4 bit TX20 wind direction values to
  // named compass directions.
  static const char* directions[] = { "N",  "NNE", "NE", "ENE", "E",  "ESE",
                                     "SE", "SEE", "S",  "SSW", "SW", "WSW",
                                     "W",  "WNW", "NW", "NNW" };

  return directions[static_cast<uint8_t>(drn)];
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// Conversion factor from seconds to microsecondss.
constexpr float k_microseconds = 1e6;

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
constexpr duration k_frame_bit_length = 0.00122 * k_microseconds;

// Frame duration in microseconds.
constexpr duration k_frame_duration = k_frame_bit_count * k_frame_bit_length;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
tx20emulator::tx20emulator(int dtr_pin, int txd_pin)
  : dtr_pin_{ dtr_pin }, txd_pin_{ txd_pin } {
  // Initialise digital pin LED_BUILTIN as an output.
}

// ------------------------------------------------------------------------------------------------
// Initialise the emulator.
// ------------------------------------------------------------------------------------------------
void tx20emulator::initialise() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // dtr needs to be pulled load for the tx20 to be active.
  pinMode(dtr_pin_, INPUT_PULLUP);

  // The frame bits are transmitted on txd.
  pinMode(txd_pin_, OUTPUT);
  digitalWrite(txd_pin_, HIGH);

  initialised_ = true;
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
// The built in led is lit white the tx20 emulator is sampling and sending.
// ------------------------------------------------------------------------------------------------
void tx20emulator::service() {

  switch (state_) {
    case tx20state::go_inactive: {

        Serial.println("go inactive");

        // led off and txd low.
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(txd_pin_, HIGH);

        state_ = tx20state::inactive;

        break;
      }

    case tx20state::inactive: {
        if (!digitalRead(dtr_pin_)) {
          t_ = micros();
          state_ = tx20state::wake_up;

          Serial.println("wakeup");
          break;
        }

        break;
      }

    case tx20state::wake_up: {
        // If dtr goes high then enter the inactive state.
        if (digitalRead(dtr_pin_))
          state_ = tx20state::go_inactive;
        else if (micros() - t_ > k_dtr_wakeup_interval) {
          t_ = micros();
          digitalWrite(LED_BUILTIN, HIGH);
          digitalWrite(txd_pin_, LOW);

          state_ = tx20state::sampling;

          Serial.println("sampling");
        }

        break;
      }

    case tx20state::sampling: {
        // If dtr goes high then enter the inactive state.
        if (digitalRead(dtr_pin_))
          state_ = tx20state::go_inactive;
        else if (micros() - t_ > k_frame_min_interval) {
          Serial.println("sending");
          state_ = tx20state::sending;
        }

        break;
      }

    case tx20state::sending: {
        int speed = static_cast<int>(sin(millis()/1000.f)*100.f + 100.f);
        write_frame(winddrn::ne, speed);
        state_ = tx20state::go_inactive;
        break;
      }
  }
}

// ------------------------------------------------------------------------------------------------
// Write a data frame to txd.
//
// Given a wind direction and speed, a tx20 frame is written to the txd pin.
// The frame consists of 41 bits which include  crc check on the data.
// ------------------------------------------------------------------------------------------------
void tx20emulator::write_frame(winddrn drn, int windspeed1) const {
  int winddrn1 = static_cast<int>(drn);

  // The second half of the frame uses inverted bits.
  // Optionally insert a bit error into the iverted bits.
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

  // That's it.
  write_txd(HIGH);
}

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
