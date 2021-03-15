# Hand vibration device API & BCI2000 Filter

## Background
From the bottom up, the speed of vibrating motors is controlled by PWM, via two Adafruit TLC5947 24 channel PWM drivers. These communicate with an Arduino via SPI protocol. The goal is to be able to control the vibrating motors on a PC, so some communication protocol must exist between the Arduino and PC. This is done via the serial port, where "packets" containing configurations and commands are sent from the PC to the Arduino. Currently, communication is uni-directional. These packets are defined in `/Vibrotactile_Firmware/command_packet.h`.

## Controlling the hand
* Upload `/Vibrotactile_Firmware/Vibrotactile_Firmware.ino` to the Arduino. Note: You must install the Arduino TLC5947 library through Arduino Library Manager.

* A C++ high-level control interface for configuring, starting, and stopping vibration is implemented in `HandVibration.h` and `HandVibration.cpp`.  

* An implementation of a BCI2000 filter is also in this repository. It uses the high-level API. Its parameters are:
  * `ActivateHandVibration` - Enable/disable control of device.
  * `COMPort` - String in the form `COM#` where `#` is the COM port number according to the device manager.
  * `SelectRandomConfig` - Select a random configuration from the `Configurations` matrix 
  * `RandConfigExpression` - Expression used to trigger vibration when `SelectRandomConfig` is enabled.
  * `Configurations` - This matrix defines all configurations of frequency and amplitude pairs. Each column is one configuration. The first row defines a [BCI2000 expression](https://www.bci2000.org/mediawiki/index.php/User_Reference:Expression_Syntax) and the second and third rows define the amplitude (in the range [0-100]) and frequency (in Hz), respectively. If `SelectRandomConfig` is enabled, the contents of the top expression row are ignored, but the row must still be present.
