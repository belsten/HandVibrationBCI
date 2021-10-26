#pragma once
#inlcude <set>
#include "SerialPort.h"
#include "Vibrotactile_Firmware/command_packet.h"

class HandVibration
{
public:
  HandVibration ();
  ~HandVibration ();

  bool Open (const std::string& _COMstring);
  void Close ();
  bool isConnected () { return m_is_connected; }

  bool ConfigureVibration (int _amplitude, float _frequency, std::set<uint8_t> locations);
  bool StartVibration ();
  bool StopVibration ();

private:
  SerialPort*              mp_arduino;
  CommandPacket            m_cmd_pkt;
  ConfigurationPacket      m_config_pkt;
  bool                     m_is_connected;
};
