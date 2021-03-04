#pragma once

#pragma pack(1)

#include "stdint.h"

typedef uint8_t CommandType;

// Commands that can be sent to the Arduino
enum Command
{
  BlinkLED       = 0,
  Configure      = 1,
  Start          = 2,
  Stop           = 3,
  NotImplemented = 4
};

// A packet that gives the Arduino a command 
struct CommandPacket
{
  CommandType Command;
};

// packet that configures relevant parameters of stimulation, is immediately sent after a "Configure" command packet
struct StimulationConfiguration {
  uint8_t    Amplitude;        // unknown
  uint16_t   Frequency;        // in 0.1Hz
};
#pragma pack()
