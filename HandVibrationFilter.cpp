////////////////////////////////////////////////////////////////////////////////
// Authors: Alex Belsten belsten@neurotechcenter.org
// Description: HandVibrationFilter implementation
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2021: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////

#include "HandVibrationFilter.h"
#include "BCIStream.h"
using namespace std;

RegisterFilter(HandVibrationFilter, 2.C3);

struct ConfigParams;
std::set<uint8_t> BCI2000ListToSet(ParamRef ch_parm, bool subtract_one);

HandVibrationFilter::HandVibrationFilter () :
  mEnable    (false),
  mVibrating (false)
{
  mDevice = HandVibration ();
}


HandVibrationFilter::~HandVibrationFilter ()
{
  if (mDevice.isConnected ())
    mDevice.Close ();
}


void
HandVibrationFilter::Publish()
{
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering:HandVibrationFilter int ActivateHandVibration= 0 0 0 1 // (boolean)",
   "Filtering:HandVibrationFilter string COMPort= COM1 % % % // COM port the Arduino is connected to",
   "Filtering:HandVibrationFilterc matrix Configurations= "
     "{ Configuration%20Expression Amplitude Frequency Locations} "  // row labels
     "{ Configuration%201 Configuration%202 Configuration%203 } " // column labels
     "StimulusCode==1  StimulusCode==2  StimulusCode==3 "         // Switch Expression
     "10 100 50 "                                                 // Amplitude
     "4 5 6 ",                                                    // Frequency
    "{ list 2 1 2 } { list 2 2 3 } { list 2 3 4 }"                // Locations
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "HandVibration 1 0 0 0",
 END_STATE_DEFINITIONS

}


void
HandVibrationFilter::AutoConfig (const SignalProperties& Input)
{
  if ((bool)OptionalParameter ("ActivateHandVibration", false))
  {

  }
}


void
HandVibrationFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input;
  if ((bool)OptionalParameter ("ActivateHandVibration", false))
  {
    GenericSignal preflightSignals (Input);
    State ("HandVibration");
    Parameter ("COMPort");
    PreflightCondition (Parameter ("Configurations")->NumRows () == 3);

    // for each configuration
    for (int config = 0; config < Parameter ("Configurations")->NumColumns (); config++ )
    {
      // check that expression is valid
      Expression switchExpression = Parameter ("Configurations")(0, config);
      switchExpression.Compile ();
      switchExpression.Evaluate (&preflightSignals);

      int amplitude_idx = 1;
      int frequency_idx = 2;
      // check that amplitude is [0-100]
      if (Parameter ("Configurations")(amplitude_idx, config) > 100 || Parameter ("Configurations")(amplitude_idx, config) < 0)
      {
        bcierr << "HandVibration Configuration error: Amplitude of configuration must be 0-100" << endl;
      }
      // check that frequency greater than zero
      if (Parameter ("Configurations")(frequency_idx, config) < 1)
      {
        bcierr << "HandVibration Configuration error: Frequency of configuration must be greater than zero" << endl;
      }
      // check that locations are valid
      // TODO
    }
  }
}


void
HandVibrationFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mEnable = (bool)OptionalParameter ("ActivateHandVibration", false);
  if (!mEnable)
    return;

  if (mDevice.isConnected ())
    mDevice.Close ();

  if (!mDevice.Open (Parameter ("COMPort").ToString ()))
  {
    bcierr << "HandVibrationFilter error: Unable to connect to device at " << Parameter ("COMPort").ToString ()
           << ". Check that device is plugged in and present in Device Manager" << endl;
  }

  // Load all necesary parameters into the ConfigurationList data structure
  mConfigList.clear ();
  // note the locations of specific configuration parameters
  int expression_idx = 0;
  int amplitude_idx  = 1;
  int frequency_idx  = 2;
  int location_idx   = 3;

  // for each configuration add to configlist
  for (int c = 0; c < Parameter ("Configurations")->NumColumns (); c++)
  {
    // make ConfigParams to store amplitude, frequency, locations
    ConfigParams configuration_parameters;
    configuration_parameters.amplitude = Parameter ("Configurations")(amplitude_idx, c);
    configuration_parameters.frequency = Parameter ("Configurations")(frequency_idx, c);
    configuration_parameters.locations = BCI2000ListToSet(Parameter ("Configurations")(location_idx, c), false);

    // assign an Expression to the pair defined above and add it to the list
    Configuration config;
    config.first  = (Expression)Parameter ("Configurations")(expression_idx, c);
    config.second = configuration_parameters;
    mConfigList.push_back (config);
  }
}


void
HandVibrationFilter::StartRun()
{
  if (!mEnable)
    return;

  // mDevice.StartVibration ();
}


void
HandVibrationFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input;
  if (!mEnable)
    return;

  if (mVibrating)
  {
    // we are currently vibrating, check if we should stop
    if (!mCurrentConfiguration->first.Evaluate(&Input))
    {
      mDevice.StopVibration ();
      State ("HandVibration") = 0;
      mVibrating = false;
    }
  }
  else
  {
    // not vibrating, check if we should start
    if (this->EvaluateConfigurations (Input))
    {

      mDevice.StartVibration ();
      State ("HandVibration") = 1;
      mVibrating = true;
    }
  }
}


void
HandVibrationFilter::StopRun()
{
  if (!mEnable)
    return;

  // mDevice.StopVibration ();
  if (mVibrating)
  {
    mDevice.StopVibration ();
    State ("HandVibration") = 0;
    mVibrating = false;
  }
}


bool
HandVibrationFilter::EvaluateConfigurations (const GenericSignal& Input)
{
  ConfigurationList::iterator itr = mConfigList.begin ();
  // go through all configuration expressions to see if any evaluate true
  while (itr != mConfigList.end ())
  {
    if (itr->first.Evaluate (&Input))
    {
      // Bingo!! Found one!
      mCurrentConfiguration = itr;

      // send the configuration to the device
      mDevice.ConfigureVibration (mCurrentConfiguration->second.amplitude,
                                  mCurrentConfiguration->second.frequency,
                                  mCurrentConfiguration->second.locations
                                 );
      return true;
    }
    itr++;
  }
  return false;
}


std::set<uint8_t> BCI2000ListToSet(ParamRef ch_parm, bool subtract_one)
{
	std::set<uint8_t> ch_set = {};
	for (int ch = 0; ch < ch_parm->NumValues(); ch++)
	{
		if(subtract_one)
			ch_set.insert( (uint8_t)ch_parm(ch) - 1);
		else
			ch_set.insert((uint8_t)ch_parm(ch));
	}
	return ch_set;
}
