#include <Arduino.h>
#include "Adafruit_TLC5947.h"
#include "command_packet.h"

// ========== PWM modules ==========
#define NUM_TLC5974 2 // number of boards chained
#define DATA        8
#define CLOCK       9
#define LATCH      10
#define oe         -1  // set to -1 to not use the enable pin (its optional)

#define N_CHANNELS 48

// ========== Global structures ==========
Adafruit_TLC5947                tlc = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);
CommandPacket               cmd_pkt;
StimulationConfiguration config_pkt = {
                                         0,    // Amplitude
                                         5.0,  // Frequency  
                                      };
bool flag_test_LED      = false;
bool flag_stimulation   = false;
bool stimulation_toggle = true;

// ========== Functions ==========

/*
 * Function to get the command type sent from the PC 
 */
void GetCommand() {
  if (Serial.available() != sizeof(CommandPacket))
  {
    Serial.readString();
    Serial.println("Sent bytes does not match packet size");
  }
  Serial.readBytes((char*)&cmd_pkt, sizeof(CommandPacket));
}


/*
 * Function to get the stimulation configuration from the PC. This function should
 * always be called after a "BlinkLED" or "Configure" command packet is received. 
 */
void GetConfiguration() {
  int count = 0;
  while (Serial.available() == 0) {
    count++;
    if (count > 1000)
    {

      return;
    }
  }
  Serial.readBytes((char*)&config_pkt, sizeof(StimulationConfiguration));
}

/*
 * Handle all necessary procedures to start the stimulation 
 */
void StartStimulation() {
  // enable interrupts
  flag_stimulation = true;
}

/*
 * Handle all necessary procedures to stop the stimulation 
 */
void StopStimulation() {
  // disable interrupts
  flag_stimulation = false;
  
  // turn off stimulation
  if (flag_test_LED) {
     // turn off LED
     digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  }
  else {
    // set PWM to zero across all channels
    
  }
  
  // set LED flag off
  flag_test_LED  = false; 

  // set the toggle such that in the next trial the stimulation is triggered first
  stimulation_toggle = true;
}

void TurnOffVibration() {
  for (int ch = 0; ch < N_CHANNELS; ch++) {
    tlc.setPWM(ch, 0);
    tlc.write(); 
  }
}

/*
 * Blink the internal LED. Useful function for debugging and demonstrating basic usage.  
*/
void ExecuteBlinkLED(int n_blinks=10) {
  int ms_on_and_off = 100;
  
  for (int i = 0; i < n_blinks; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    delay(ms_on_and_off);              // wait 
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
    delay(ms_on_and_off);              // wait 
  }
}
 

/*
 * Turn on and off the LED with each iterrupt
 */
ISR(TIMER1_COMPA_vect) {  
  if (flag_stimulation) {
    if (true/*test_LED*/) {
      if (stimulation_toggle) {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
      }
      else{
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
      }  
    }
    else {
      
    }
    stimulation_toggle = !stimulation_toggle;
  }
}


/*
 * Configure timer1 for interrupts
 */
void ConfigureTimer1() {
  noInterrupts();  //stop interrupts
  
  int prescaler = 1024;
  
  // set timer1 interrupt
  TCCR1A = 0;           // set entire TCCR1A register to 0
  TCCR1B = 0;           // same for TCCR1B
  TCNT1  = 0;           // initialize counter value to 0
  
  // set compare match register
  float freq = 4;
  OCR1A = (uint16_t)((16/freq)*((float)1000000/(float)prescaler)) - 1;
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);

  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}


// ========== setup ==========
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial);
  TurnOffVibration();
  ConfigureTimer1();  
}

// ========== loop ==========
void loop() {
  // put your main code here, to run repeatedly:
  
  if (Serial.available() > 0) {
    
    // read the incoming command
    GetCommand();
    // decide what to do with the new command
    switch (cmd_pkt.Command) {
      case BlinkLED:
        // ============== BLINK CMD ==============
        flag_test_LED = true;
        // ExecuteBlinkLED();
        // Serial.print("BLINK");
        break;
     case Configure:
        // ============== CONFIGURE CMD ==============
        // get the stim configuration
        GetConfiguration();
        // apply the configuration
        ConfigureTimer1();
        break;
     case Start:
        // ============== START CMD ==============
        StartStimulation();
        // ExecuteBlinkLED(20);
        //Serial.print("STARTED");
        break;
     case Stop:
        // ============== STOP CMD ==============
        StopStimulation();
        // ExecuteBlinkLED(5);
        // Serial.print("STOPPED");
        break;
     default:
        //Serial.print("UNKOWN");
        break;
    }
    /*
    // say what you got:
    Serial.print("I received: ");
    Serial.println(cmd_pkt.Command , DEC);
    */
  }
}
