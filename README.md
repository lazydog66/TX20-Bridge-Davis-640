# TX20Bridge6410
This project is about making an interface for a Davis 6410 anemometer so that it can be used in place of a Technoline TX20 or LaCross TX20. It contains a few classes demonstrating how to interface to a Davis 6410 anemometer and wind vane, and another for emulating a Technoline/La Cross TX20.

 Some time ago I built and coded an Arduino based wind station that logs and reports wind speed and direction to a Raspberry PI server. The wind station runs on an Ateml1284p, and the communications side is handled by a SIM800L modem. The station used a TX20 wind sensor to measure the wind speed and direction.  Recently however, the TX20 started failed due to moisture in the TX20 unit. I needed to replace the TX20, but sadly, that particular model was no longer available and I had to think about an alternative. After looking at various other makes, the Davis 6410 caught my eye. It looks similar to the TX20 but works on a simpler design (a reed or hall effect switch for the anemometer, and a potentiometer for the wind vane). The TX20 on the other hand is a bit more complicated as it houses its own processor inside the unit. The electronics reads the wind speed and direction and then encodes the data before sending it down the line as a serial data stream. If I was going to replace the TX20 with the Davis 6410 then I would need to change the hardware and software on my wind station, something I wasn't too keen on doing. Then I had a thought, why not leave the wind station hardware and software as it is, and instead convert the Davis 6410 into a TX20. This is what I ended up doing.

## The Bridging Box
To convert the Davis 6410 into a TX20, I built a 'bridging' box. The box comprises an input socket which the Davis6410 plugs into, and an output socket which emulates the old TX20 unit. The box does two things, it converts the physical connections for the TX20 cable into those for the Davis 6410. It also reads the data from the Davis 6410 and encodes the data into the format emitted by the TX20. The bridge turned out to be very easy to implement with just an Arduino Pro Min 328. This is how it works.

The connection to the TX20 uses 4 wires, Vcc, GND, Dtr, TXd, and these lines go straight into the bridging box. The two power lines are used to power the Arduino directly (the Arduino Pro Mini uses just a few milliamps). Dtr and TXd lines also go directly to the Arduino (Dtr is an input to the Arduino and Txd is an output from the Arduino). The purpose of these two lines is as follows. Dtr is used to control the TX20. When it is pulled low the TX20 reads the wind speed and direction and transmits the values on TXd. Releasing Dtr stops the TX20 from sending data on TXd. I couldn't find official information on the TX20 protocol but when I built the original wind station I used the information I found on [this](https://app.grammarly.com/ddocs/1246441284) website to write code to decode the TX20 data stream. For the bridge box, all I had to do was the reverse action, ie convert wind speed and direction to the TX20 data stream.

One note of caution, if you do build an emulator based on this project, please make sure that the voltage levels on the physical connections are correct. For my project, it was easy because I built all the hardware myself, including the wind station. I knew exactly what levels were being used. If you are using a commercial wind station then you will need to find this information out first before building a bridging box.

## Code Branches
There are two code branches in this repository, *master* and *speed-on-adc*. The first implements reading the wind speed signal from the Davis 6410 using a hardware falling edge interrupt on the Arduino. The other branch, *speed-on-adc*, reads the speed using the hardware adc. When I implemented the falling edge method, I found that, under certain circumstances, the signal coming from the Davis 6410 needed filtering. This can be done using a resistor and capacitor on the signal line feeding into the Arduino, but I found it more convenient to do the filtering in software. This was easily done by connecting the signal line to an adc channel. If you are looking at using a Davis 6410, then take a look at the branch which suits your needs. 

## The Code
The code for the bridge comprises two main classes, *davis6410* and *tx20emulator*. The first handles reading the anemometer and wind vane on the Davis 6410. The other converts a wind speed and direction to a TX20 data frame. The *led* class is a simple way of blinking an LED to let me know that the bridge is working.

I used *PlatformIO* to develop the bridge software. I like it because it integrates nicely with *Visual Studio Code* which is a very nice IDE in my opinion. If you prefer to use the Arduino IDE then I don't think you will have much trouble taking the *.h* and *.cpp* files and creating an Arduino project from them.

The important classes are, *davis6410* and *tx20emulator*. The first is responsible for reading the Davis 6410 and reporting the wind speed and direction. If you're reading this and only interested in the  Davis 6410 part of the project, then this class can be lifted and used in your project. Class *tx20emulator* contains the code that allows an Arduino Pro Mini to emulate a TX20.

### class davis6410
The Davis 6410 is hooked up to the Arduino using A2 for the anemometer, and A0 for the wind vane. Each revolution of the wind cups engages the reed switch in the anemometer causing a pulse. The spec for the 6410 gives a simple formula for calculating the wind speed in miles per hour from the number of pulses and a given period of time. The formula is,
```
V = P*T/2.25
```
Here, T is the sample period and P is the number of pulses (wind cup revolutions) from the anemometer. If you take your sampling period to be 2.25 seconds, then the number of pulses equates nicely to the wind speed in miles per hour. Another advantage of using 2.25 seconds is that the pulse counter variable needs only to be an 8-bit value (I'm not going to worry about trying to measure a 255+ mph wind).

*davis6410* is implemented as a state machine driven by the method *service()*. After creating a *davis6410*. It should be called from within the main loop as quickly as possible. To initiate a new wind sample,call *start_sample()*. The service routine will then count pulses and when the sample period is over, the results are reported. Results are reported using a callback mechanism which is passed in when *start_sample* is called. Only one sample is taken at a time, so to keep sampling you need to call *start_sample()* repeatedly.

### class adctask
 Class *adctask* is the base class for reading the analog channels. The task runs in the background at about 5KHz care of timer 1 interrupt. 

### class adcpulse and movingaverage
To count the anemometer pulses, A2 is read and the signal filtered. Classes *adcpulse* and *movingaverage* do the reading and filtering.

### class adctaskaverage
 The wind vane is read by class *adctaskaverage*.

### class pulsegenerator
The class *pulsegenerator* was used to help with testing. It generates a pulse on a digital
pin and can be used to emulate the speed signal coming from the davis 6410. 

### class tx20emulator
This class emulates the Dtr and Txd lines of a TX20 on two Arduino pins. The emulator is implemented as a simple state machine and driven by the service routine *service()*. The Dtr line uses a digital io pin with the internal pullup resistor enabled. The idea is that whatever is attached to Dtr must pull the line low to enable the TX20 emulator. The emulator uses another digital io pin to implement TXd. When Dtr is low, the emulator is active and will sample the wind speed and direction and then encode the results and send the data on TXd. It's difficult to know exactly how the TX20 behaves exactly when Dtr changes state in the middle of sending a data frame etc, hence the emulator might not mimic the behaviour of a real TX20 all the time.

### windmeterintf
This is an interface class between *tx20emulator* and a wind meter. The idea is to make it easy for the emulator to work with other wind meters and not just the Davis 6410.

### led
This is a simple class for controlling an led. It's not needed but I added it so that I could add a flashing led to my project. The led flashes every time the emulator sends a TX20 data frame.

## Conclusion
This project solves a specific problem I had, namely how to replace a broken TX20 wind meter with a Davis 6410. It also provides a couple of classes which you may find useful, namely *tx20emulator* which turns two pins of an Arduino Pro Min into a *TX20*, and *davis6410* which can be used to interface to a Davis 6410 wind meter.

