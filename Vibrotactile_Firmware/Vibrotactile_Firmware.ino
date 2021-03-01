#include <Arduino.h>
#include "Adafruit_TLC5947.h"
#include "command_packet.h"

// ========== PWM modules ==========
#define NUM_TLC5974 2 // number of boards chained
#define DATA        8
#define CLOCK       9
#define LATCH      10
#define oe         -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, CLOCK, DATA, LATCH);
CommandPacket pkt;

// ========== Functions ==========
void GetCommand() {
  Serial.readBytes((char*)&pkt, sizeof(CommandPacket));
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
    delay(ms_on_and_off0);             // wait 
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
    delay(ms_on_and_off);              // wait 
  }
}

// ========== setup ==========
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial);
}

// ========== loop ==========
void loop() {
  // put your main code here, to run repeatedly:
  
  if (Serial.available() > 0) {
    
    // read the incoming command
    GetCommand();

    // decide what to do with the new command
    switch (pkt.Command) {
      case BlinkLED:
        // Blink the internal LED
        ExecuteBlinkLED();
        break;
     case Configure:
        ParseConfiguration
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
    Serial.println(pkt.Command , DEC);
  }
}
