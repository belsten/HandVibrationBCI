#include <Arduino.h>
#include "Adafruit_TLC5947.h"
#include "command_packet.h"

// ========== PWM modules ==========
#define NUM_TLC5974 2 // number of boards chained
#define DATA        8
#define CLOCK       9
#define LATCH      10
#define oe         -1  // set to -1 to not use the enable pin (its optional)

// ========== Global structures ==========
Adafruit_TLC5947                tlc = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);
CommandPacket               cmd_pkt;
StimulationConfiguration config_pkt;

bool stimulation_toggle = false;

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
  Serial.readBytes((char*)&config_pkt, sizeof(StimulationConfiguration));
}


/*
 * Blink the internal LED. Useful function for debugging and demonstrating basic usage.  
 */
void ExecuteBlinkLED() {
  int n_blinks      = 15;
  int ms_on_and_off = 100;
  
  for (int i = 0; i < n_blinks; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    delay(ms_on_and_off);             // wait 
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
    delay(ms_on_and_off);              // wait 
  }
}


/*
 * Turn on and off the LED with each iterrupt
 */
ISR(TIMER1_COMPA_vect) {  
  if (stimulation_toggle){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  }
   stimulation_toggle = !stimulation_toggle;
}


/*
 * Configure timer1 for interrupts
 */
void ConfigureTimer1() {
  cli();  //stop interrupts
  int prescaler   = 256;
  float frequency = 1;
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;           // set entire TCCR1A register to 0
  TCCR1B = 0;           // same for TCCR1B
  TCNT1  = 0;           // initialize counter value to 0
  
  // set compare match register
  OCR1A = (16*10^6) / (frequency*prescaler) - 1; // (must be <65536)
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts  
}


// ========== setup ==========
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial);
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
      
        // get the stim configuration
        GetConfiguration();
        ExecuteBlinkLED();
      
        break;
     case Configure:
      
        // get the stim configuration
        GetConfiguration();
        
        break;
     case Start:
        Serial.print("Start");
        break;
     case Stop:
        Serial.print( "Stop");
        break;
      default:
        Serial.print("Unknown command");
    }
    
    // say what you got:
    Serial.print("I received: ");
    Serial.println(cmd_pkt.Command , DEC);
  }
}
