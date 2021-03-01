#include <iostream>
#include <chrono>
#include <thread>

#include "SerialPort.h"
#include "Vibrotactile_Firmware/command_packet.h"

const char* portName = "\\\\.\\COM6";

//Declare a global object
SerialPort*               arduino;
CommandPacket             cmd_pkt;
StimulationConfiguration  config_pkt;
int main (void)
{
  std::cout << "Attempting to connect..." << std::endl;
  arduino = new SerialPort (portName);
  std::cout << "Is connected: " << arduino->isConnected () << std::endl;

  if (arduino->isConnected ()) 
  {
    bool hasWritten = false;
    for (int i = 0; i < 2; i++)
    {
      // send command to blink LED
      cmd_pkt.Command = BlinkLED;

      hasWritten = arduino->writeSerialPort ((char*)& cmd_pkt, sizeof (CommandPacket));
      if (hasWritten) std::cout << "Command Written Successfully" << std::endl;
      else std::cerr << "Command was not written" << std::endl;

      // send the configuration
      config_pkt.Amplitude = 100;
      config_pkt.Frequency = 1;

      hasWritten = arduino->writeSerialPort ((char*)& config_pkt, sizeof (StimulationConfiguration));
      if (hasWritten) std::cout << "Configuration Written Successfully" << std::endl;
      else std::cerr << "Configuration was not written" << std::endl;

      // sleep for 5 seconds
      std::this_thread::sleep_for (std::chrono::milliseconds (5000));
    }
  }
  return 1;
}
