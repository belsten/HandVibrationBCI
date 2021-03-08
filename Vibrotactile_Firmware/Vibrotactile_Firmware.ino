#include <Arduino.h>
#include "Adafruit_TLC5947.h"
#include "command_packet.h"

// ========== PWM modules ==========
#define NUM_TLC5974 2 // number of boards chained
#define DATA        8
#define CLOCK       9
#define LATCH      10
#define oe         -1  // set to -1 to not use the enable pin (its optional)

#define N_MOTORS         48
#define CLOCK_RATE 16000000.0

// ========== Global structures ==========
Adafruit_TLC5947                tlc = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);
CommandPacket               cmd_pkt;
StimulationConfiguration config_pkt;
                                 
bool flag_test_LED      = false;    // This flag is set high if the BlinkLED command is received
bool flag_stimulation   = false;    // This flag is set according to the Start and Stop commands
bool stimulation_toggle = true;     // This flag toggles stimulation on and off during stimulation

// ========== Functions ==========

/*
 * Function to get the command type sent from the PC 
 */
void GetCommand() {
  Serial.readBytes((char*)&cmd_pkt, sizeof(CommandPacket));
}

/*
 * Function to get the stimulation configuration from the PC. This function should
 * always be called after a "BlinkLED" or "Configure" command packet is received. 
 */
void GetConfiguration() {
  /*
  int count = 0;
  while (Serial.available() == 0) {
    count++;
    if (count > 1000) {
      return;
    }
  }
  */
  
  Serial.readBytes((char*)&config_pkt, sizeof(StimulationConfiguration));
  if (config_pkt.Amplitude > 100) config_pkt.Amplitude =  100;
  if (config_pkt.Amplitude <   0) config_pkt.Amplitude =    0;
  if (config_pkt.Frequency <=  0) config_pkt.Frequency =    1;
}

/*
 * Handle all necessary procedures to start the stimulation 
 */
void StartStimulation() {
  flag_stimulation = true;
}

/*
 * Handle all necessary procedures to stop the stimulation 
 */
void StopStimulation() {
  noInterrupts();
  flag_stimulation = false;

  TurnOffVibration();
  interrupts();
  /*
  // turn off stimulation
  if (flag_test_LED) {
     // turn off LED
     digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
     // set LED flag off
     flag_test_LED  = false; 
  }
  else {
     // set PWM to zero across all channels
     TurnOffVibration();
  }
  */


  // set the toggle such that in the next trial the stimulation is triggered first
  stimulation_toggle = true;
}

/* 
 *  Turn on vibration for all motors
 */
void TurnOnVibration() {
  // The PWM value in the API is [0-4095]
  // Scale amplitude (which is a uint8_t in [0-100]) to a value in this range
  uint16_t scaled_amplitude = ((4095.0/100.0)*(float)config_pkt.Amplitude);
  
  for (int ch = 0; ch < N_MOTORS; ch++) {   
    tlc.setPWM(ch, scaled_amplitude);
  }
  tlc.write(); 
}


/* 
 *  Turn off vibration for all motors
 */
void TurnOffVibration() {
  for (int ch = 0; ch < N_MOTORS; ch++) {
    tlc.setPWM(ch, 0);
  }
  tlc.write(); 
}

/*
 * Blink the internal LED. Useful function for debugging and demonstrating basic usage. 
*/
void ExecuteBlinkLED(int n_blinks=10) {
  int ms_on_and_off = 100;
  
  for (int i = 0; i < n_blinks; i++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    delay(ms_on_and_off);              // wait 
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
    delay(ms_on_and_off);              // wait 
  }
}
 

/*
 * Turn on and off the stimulation with each interrupt
 */
ISR(TIMER1_COMPA_vect) {  
  if (flag_stimulation) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    if (stimulation_toggle) {                 
        TurnOnVibration();                 // turn on vibration
    }
    else {
        TurnOffVibration();                // turn off vibration
    }
    stimulation_toggle = !stimulation_toggle;  
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off
  }
}


/*
 * Configure timer1 for interrupts. Timer1 is desirable becuase it is a 16-bit timer,
 * as opposed to Timer0 or 2, which are 8-bit timers. 
 * 
 * The prescaler can be 1, 8, 64, 256, or 1024. It controls how often the timer register 
 * is incremented. The register increments according to the clock rate divided by the 
 * prescaler (16 MHz / prescaler)
 */
void ConfigureTimer1() {
  noInterrupts();  //stop interrupts
  
  int prescaler = 1024;
  
  // set timer1 interrupt
  TCCR1A = 0;           // set entire TCCR1A register to 0
  TCCR1B = 0;           // same for TCCR1B
  TCNT1  = 0;           // initialize counter value to 0
  
  // Set compare match register
  // When the timing register is equal to this value, the Timer1 callback function will
  // be called. 
  OCR1A = (uint16_t)(CLOCK_RATE/((float)(config_pkt.Frequency)*0.1*(float)prescaler)) - 1;
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);

  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}


// ========== setup ==========
/*
 * This function is called when the device is powered on or reset.
 * Initialization of PWM boards, timer, Serial port, and LED takes
 * place here. 
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial);
  tlc.begin();
  TurnOffVibration();

  config_pkt.Frequency = 10;
  config_pkt.Amplitude = 10;
  ConfigureTimer1();  
}

// ========== loop ==========
/*
 * This function is called repetedly after setup. It handles incoming
 * commands on the serial port.
 */
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

        break;
     case Stop:
        // ============== STOP CMD ==============
        StopStimulation();

        break;
     default:
        //Serial.print("UNKOWN");
        break;
    }
  }
}
