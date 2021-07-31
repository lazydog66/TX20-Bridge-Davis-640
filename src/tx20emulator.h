// ------------------------------------------------------------------------------------------------
// This is the class for the tx20 emulator
//
// It turns pins k_data_dtr k_data_txt_out ino the input and outputs of a tx20 unit.
// k_data_dtr should be taken low to make the emulator active, just like a real
// tx20. When k_data_dtr is low, the emulator starts sending data frames on
// k_data_txd_out every few seconds.
// ------------------------------------------------------------------------------------------------
#pragma once

#include <Arduino.h>

// Default to using pin 3 for the Dtr line.
constexpr int k_dtr_pin = 3;

// Default to using pin 4 for the Txd line.
constexpr int k_txd_pin = 4;

// These are the states the tx20 emulator can be in.
enum class tx20state  {
  nothing,
  disabled,
  start_sample,
  sampling,
  sending
};

// Durations are measured in microseconds.
using duration = uint32_t;

// windmeterintf is an interface class  for wind meters.
class windmeterintf;

// Utility function to convert a wind direction value to a name string.
const char* winddrn_to_string(int drn);

class tx20emulator {
public:
  tx20emulator(int dtr_pin = k_dtr_pin, int txt_pin = k_txd_pin);

  // Initialise the resources used by the emulator.
  // Must be done before the eumlator can be used.
  void initialise(windmeterintf* wind_meter);

  // Service the tx20 emulator.
  // This should be called periodically,
  void service();

  // Return the state of the tx20 emulator.
  tx20state state() const { return state_; }

private:

  // Set the internal state of the tx20 emulator.
  // This may send commands to the attached wind meter and set the state of any leds.
  void set_state(tx20state state);

  // Write a data frame to Txd.
  // See above for details on the bit layout of the frame.
  void write_frame(float mph, int direction) const;

  // Read the input level of Dtr.
  // A low enables the tx20 and high disables it.
  bool read_dtr() const;

  // Writes a value to TxD.
  void write_txd(bool value) const;

  // If the dtr pin is held low, the tx20 emulator starts sampling and sending frames.
  const int dtr_pin_;

  // This pin is used to send the frame over.
  // See the .cpp file for a description of the bits tha tmake up the frame.
  const int txd_pin_;

  // Will be true if the emulator has been initialised.
  bool initialised_ = false;

  // The attached wind meter.
  // The wind meter is abstracted away by the interface class windmeterintf.
  windmeterintf* wind_meter_ = nullptr;

  // The emulator is implemented as a state machine.
  tx20state state_ = tx20state::nothing;

  // General purpose timer value.
  duration t_;
};
