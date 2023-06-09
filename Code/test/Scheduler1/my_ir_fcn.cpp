#include "my_ir_fcn.h"     
#include "Arduino.h"  


int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

const int PIN_TO_SENSOR = 34;

// esempio di passaggio tra interno al flie e mondo esterno 
void IR_Tsk(int& pinStatePrevious) 
{
  pinStatePrevious = pinStateCurrent; // store old state
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // pin state change: LOW -> HIGH
    Serial.println("Motion detected!");
    // TODO: turn on alarm, light or activate a device ... here
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
  }

}

void IR_PwrOn(void)
{
  // definizione tipologia pin
  pinMode(PIN_TO_SENSOR, INPUT);
  // condizione iniziale IR
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);
}