// ------------------------------------------------------------------------------------------------
// This is the class for the tx20 emulator
//
// It turns pins k_data_dtr k_data_txt_out ino the input and outputs of a tx20 unit.
// k_data_dtr should be taken low to make the emulator active, just like a real
// tx20. When k_data_dtr is low, the emulator starts sending data frames on
// k_data_txd_out every few seconds.
// ------------------------------------------------------------------------------------------------

#pragma once

enum class winddrn : uint8_t {
  n,
  nne,
  ne,
  ene,
  e,
  ese,
  se,
  see,
  s,
  ssw,
  sw,
  wsw,
  w,
  wnw,
  nw,
  nnw
};

// These are the states the tx20 emulator can be in.
enum class tx20state : uint8_t {
  go_inactive,
  inactive,
  wake_up,
  sampling,
  sending

};

// Durations are measued in microseconds.
using duration = uint32_t;

// Utility function to convert a wind direction value to a name string.
const char* winddrn_to_string(winddrn drn);

class tx20emulator {
public:
  tx20emulator(int dtr_pin, int txt_pin);

  // Initialise the resources used by the emulator.
  // Must be done before the eumlator can be used.
  void initialise();

  // Service the tx20 emulator.
  // This should be called periodically,
  void service();

private:
  // Write a data frame to Txd.
  // See above for details on the bit layout of the frame.
  void write_frame(winddrn winddrn1, int windspeed1) const;

  // Read the current data value of TxD.
  // This is used by instances athat are controllers.
  bool read_txd() const;

  // Writes a value to TxD.
  void write_txd(bool value) const;

  // If the dtr pin is held low, the tx20 emulator starts sampling and sending frames.
  int dtr_pin_;

  // This pin is used to send the frame over.
  // See the .cpp file for a description of the bits tha tmake up the frame.
  int txd_pin_;

  // Will be true if the emulator has been initialised.
  bool initialised_ = false;

  // The emulator is implemented as a state machine.
  tx20state state_ = tx20state::go_inactive;

  // General purpose timer value.
  duration t_;
};
