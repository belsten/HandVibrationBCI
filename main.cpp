#include <iostream>
#include <chrono>
#include <thread>

#include "SerialPort.h"
#include "Vibrotactile_Firmware/command_packet.h"


#define MAX_DATA_LENGTH 255

char incomingdata[MAX_DATA_LENGTH];
const char* portName = "\\\\.\\COM6";

//Declare a global object
SerialPort*               arduino;
CommandPacket             cmd_pkt;
StimulationConfiguration  config_pkt;


void parseResponse (char* _response) {
  std::cout << std::string(_response) << std::endl;
  /*
  if (_response[0] == BlinkLED)
  {
    std::cout << "GOT BLINK RESPONSE" << std::endl;
  }
  else if (_response[0] == Configure)
  {
    std::cout << "GOT CONFIG RESPONSE" << std::endl;
  }
  else if (_response[0] == Start)
  {
    std::cout << "GOT START RESPONSE" << std::endl;
  }
  else if (_response[0] == Stop)
  {
    std::cout << "GOT STOP RESPONSE" << std::endl;
  }
  else if (_response[0] == NotImplemented)
  {
    std::cout << "GOT NotImplemented  RESPONSE" << std::endl;
  }
  else {
    std::cout << "GOT ???????" << std::endl;
  }
  */
}


int main (void)
{
  std::cout << "Attempting to connect..." << std::endl;
  arduino = new SerialPort (portName);
  std::cout << "Is connected: " << arduino->isConnected () << std::endl;
  std::cout << "size of cmd packet "<< sizeof(StimulationConfiguration) << std::endl;
  if (arduino->isConnected ()) 
  {
    bool hasWritten = false;
    
    for (int i = 0; i < 3; i++)
    {
      std::this_thread::sleep_for (std::chrono::milliseconds (1000));
      // send command to blink LED
      cmd_pkt.Command = BlinkLED;
      hasWritten      = arduino->writeSerialPort ((char*)& cmd_pkt, sizeof (CommandPacket));
      if (hasWritten) std::cout << "Blink Command Written Successfully" << std::endl;
      else            std::cerr << "Command was not written"            << std::endl;

      /*
      // send command to configure
      cmd_pkt.Command = Configure;
      hasWritten = arduino->writeSerialPort ((char*)& cmd_pkt, sizeof (CommandPacket));
      if (hasWritten) std::cout << "Configure command Written Successfully" << std::endl;
      else std::cerr << "Command was not written" << std::endl;

      // send the configuration
      config_pkt.Amplitude = 100;
      config_pkt.Frequency = 5;

      hasWritten = arduino->writeSerialPort ((char*)& config_pkt, sizeof (StimulationConfiguration));
      if (hasWritten) std::cout << "Configuration Written Successfully" << std::endl;
      else std::cerr << "Configuration was not written" << std::endl;
       */
      
      // send command to start
      cmd_pkt.Command = Start;
      hasWritten      = arduino->writeSerialPort ((char*)& cmd_pkt, sizeof (CommandPacket));
      if (hasWritten) std::cout << "Start command Written Successfully.... should be blinking" << std::endl;
      else            std::cerr << "Command was not written"                                   << std::endl;
      
      // sleep for x seconds
      std::this_thread::sleep_for (std::chrono::milliseconds (11000));
      
 
      // send command to stop
      cmd_pkt.Command = Stop;
      hasWritten      = arduino->writeSerialPort ((char*)& cmd_pkt, sizeof (CommandPacket));
      if (hasWritten) std::cout << "Stop Command Written Successfully" << std::endl;
      else            std::cerr << "Command was not written"           << std::endl;
  
      
      // sleep for x seconds
      std::this_thread::sleep_for (std::chrono::milliseconds (7000));
     
    }
  }
  delete arduino;
  return 1;
}
