#include <iostream>
#include "SerialPort.h"
#include "Vibrotactile_Firmware/command_packet.h"

const char* portName = "\\\\.\\COM6";

//Declare a global object
SerialPort* arduino;
CommandPacket pkt;

int main (void)
{
  std::cout << "Attempting to connect..." << std::endl;
  arduino = new SerialPort (portName);
  std::cout << "Is connected: " << arduino->isConnected () << std::endl;

  if (arduino->isConnected ()) 
  {
    // send command to blink LED
    pkt.Command = BlinkLED;

    bool hasWritten = arduino->writeSerialPort ((char*)& pkt, sizeof (CommandPacket));
    if (hasWritten) std::cout << "Data Written Successfully" << std::endl;
    else std::cerr << "Data was not written" << std::endl;
  }
  return 1;
}
