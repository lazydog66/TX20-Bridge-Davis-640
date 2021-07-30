// --------------------------------------------------------------------------------------------------------------------
// Interface for the Davis 6410 wind meter.
//
// The wind meter measures wind speed and direction. The speed is measured by counting pulses prodcued by a
// reed switch, and the direction is encoded by a 20KOhm potentiometer.
// --------------------------------------------------------------------------------------------------------------------

// The default pin for the wind sensor.
// Pin 2 is used because falling edge interrupts are used.
constexpr int k_wind_sensor_pin = 2;

// The default pin for the wind vane.
// The pin must be an analogue pin.
constexpr int k_wind_direction_pin = A0;

// This is the default duration over which the wind speed is calculated.
// The anenometer's spec says the minimum wind speed is 1 mph which is 1
// revolution per 2.25 seconds, so 2.25 seconds seems like a reasonable amount
// of time. This has the advantage that the returned wind speed will be an
// integer number of 1 mph.
constexpr unsigned long k_wind_speed_sample_t = 2250;

class davis6410 {
 public:
  // The Davis runs off two pins, a digital input for the wind speed pulses and
  // an analogue pin for the wind direction. The anenometer's spec says the
  // minimum wind speed is 1 mph which is 1 revolution per 2.25 seconds, so
  // 2.25 seconds for the period has the advantage that the returned pulse count
  // is the wind speed in mph.
  davis6410(int wind_sensor_pin = k_wind_sensor_pin,
            int wind_direction_pin = k_wind_direction_pin,
            unsigned long sample_period = 2250);

  // Initialise the hardware resources and set up the isr.
  // This must be done once before the 6410 can be used.
  void initialise();

  // Sample the wind speed.
  // The pulses from the Davis are counted over the sampling period and
  // converted to mph.
  float sample_wind_mph() const;

  // Sample the wind direction.
  // Returns (0, 15) where 0 is North, 4 is East etc.
  int sample_wind_direction() const;

 private:
  // Convert pulses to mph.
  // Note, this may in the future apply calibration data to the result.
  float calculate_wind_mph(uint8_t pulses) const;

  // A digital pin is used to counting the anenometer pulses.
  const int wind_speed_pin_;

  // The analog pin to use for reading the wind vane direction.
  const int wind_vane_pin_;

  // The duration in milliseconds of the sample period.
  unsigned long sample_period_;

  // The resources must be initialised before the 6410 can be read.
  bool initialised_ = false;
};
