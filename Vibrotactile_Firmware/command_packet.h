#pragma once
#pragma pack(1)

#include "stdint.h"

typedef uint8_t CommandType;

// Commands that can be sent to the Arduino
enum Command
{
  BlinkLED  = 0,
  Configure = 1,
  Start     = 2,
  Stop      = 3
};

// A packet that gives the Arduino a command 
typedef struct
{
  CommandType Command;
} CommandPacket;

// packet that configures relevent parameters of stimulation, is immediatly sent after a "Configure" command packet
typedef struct {
  uint8_t Amplitude;
  float   Frequency;
} StimulationConfiguration;

#pragma pack()
