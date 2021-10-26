#include "HandVibration.h"

HandVibration::HandVibration () :
  mp_arduino(NULL),
  m_is_connected(false)
{
  m_cmd_pkt    =  { Start };
  m_config_pkt = { 10, 10 };
}

HandVibration::~HandVibration ()
{
  this->Close ();
}

bool HandVibration::Open (const std::string& _COMstring)
{
  std::string COMprefix = "\\\\.\\";
  mp_arduino =  new SerialPort ( (COMprefix+_COMstring).c_str() );
  m_is_connected = mp_arduino->isConnected ();
  return m_is_connected;
}


void HandVibration::Close ()
{
  if (m_is_connected)
    mp_arduino->closeSerial ();
  delete mp_arduino;
  mp_arduino = NULL;
}


bool HandVibration::ConfigureVibration (int _amplitude, float _frequency, std::set<uint8_t> locations)
{
  if (m_is_connected)
  {
    m_cmd_pkt.Command = Configure;
    if (mp_arduino->writeSerialPort ((char*)& m_cmd_pkt, sizeof (CommandPacket)))
    {
      // write amp/freq
      m_config_pkt.Amplitude = _amplitude;
      m_config_pkt.Frequency = _frequency*10;
      // write locations
      m_config_pkt.Locations[5] = {0,0,0,0,0}
      int i = 0;
      std::set<uint8_t>::iterator it;
      for (it = locations.begin(); it != locations.end(); ++it) {
          m_config_pkt.Locations[i]= *it;
          i+=1;
      }
      if (mp_arduino->writeSerialPort ((char*)& m_config_pkt, sizeof (ConfigurationPacket)))
        return true;
    }
  }
  return false;
}


bool HandVibration::StartVibration ()
{
  if (m_is_connected)
  {
    m_cmd_pkt.Command = Start;
    return mp_arduino->writeSerialPort ((char*)& m_cmd_pkt, sizeof (CommandPacket));
  }
}


bool HandVibration::StopVibration ()
{
  if (m_is_connected)
  {
    m_cmd_pkt.Command = Stop;
    return mp_arduino->writeSerialPort ((char*)& m_cmd_pkt, sizeof (CommandPacket));
  }
}
