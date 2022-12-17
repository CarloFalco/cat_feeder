#include <Arduino.h>
#include <Servo.h>

#include <Wire.h>
#include <SPI.h>
#include <SD.h>


#include "Adafruit_INA219.h"
#include "SolarTrack.h"
#include "LedTogle.h"
#include "powerMeter.h"

unsigned long Count500ms;
unsigned long PreviousCount500ms = 0;

unsigned long Count1s;
unsigned long PreviousCount1s = 0;

void setup() {  

  // Initialize the Serial comunication.
  Serial.begin(115200);
  
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);  }

  Serial.println("serial port inizialized");  



  //ledTogle_PwrOn();
  delay(50);
  moveAxes_PwrOn();
  delay(50);
  powerMeter_PwrOn();

}


void loop() {

  // TASK 500ms

  Count500ms = millis();
  if (Count500ms - PreviousCount500ms > 500)
  { 
    PreviousCount500ms = Count500ms;  
    moveAxes_tsk();
  }


  // TASK 1s
  Count1s = millis();
  if (Count1s - PreviousCount1s > 1000)
  {
    PreviousCount1s = Count1s;
    powerMeter_tsk();
  }
   
  
}
