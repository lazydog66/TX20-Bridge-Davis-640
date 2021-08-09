// ------------------------------------------------------------------------------------------------
//
// This an interface class for wind meters.
//
// The idea is that is abstracts away the type of the wind meter used so that
// the tx20 emulator can work with different wind meters other than the Davis 6410.
//
// If you want to use a different wind meter with the emulator, then your class needs
// to implement start_sample(), abort_sample(), get_wid_mph() and get_wind_direction().
// ------------------------------------------------------------------------------------------------
#pragma once

// This is the callback function signature for when a sample has been taken.
using windsamplefn = void (*)(void* context);

class windmeterintf
{

public:

  // Start a new sample.
  // The callback will be called when the sample is ready.
  // Returns true if the sample was started, false otherwise.
  virtual bool start_sample(windsamplefn fn, void* context) = 0;

  // Abort the current sample if there is one in progress.
  virtual void abort_sample() = 0;

  // Return the last sampled wind speed.
  virtual float get_wind_mph() const = 0;

  // Return the last sampled wind direction.
  // Returns the direction as 0=N, E=4 etc.
  virtual int get_wind_direction() const = 0;

};

