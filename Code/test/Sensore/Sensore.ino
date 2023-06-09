
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

//  pins
const int RedLED = 12;
const int GreenLED = 14;
const int PIN_TO_SENSOR = 34;

int x = 0;

int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

void setup() {
  Serial.begin(115200); 
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  pinMode(PIN_TO_SENSOR, INPUT);
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);

  xTaskCreatePinnedToCore(
                    Task1code,   // Task function.
                    "Task1",     // name of task.
                    10000,       // Stack size of task
                    NULL,        // parameter of the task
                    1,           // priority of the task 
                    &Task1,      // Task handle to keep track of created task
                    1);          // pin task to core 0                 
  
  xTaskCreatePinnedToCore(
                    Task2code,   // Task function.
                    "Task2",     // name of task. 
                    10000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    2,           // priority of the task 
                    &Task2,      // Task handle to keep track of created task 
                    1);          // pin task to core 1

  xTaskCreatePinnedToCore(
                    Task3code,   // Task function.
                    "Task3",     // name of task. 
                    10000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    3,           // priority of the task 
                    &Task3,      // Task handle to keep track of created task 
                    1);          // pin task to core 1
}

//Task1code: blinks an LED every 700 ms
void Task1code( void * pvParameters ){
  while(1)
  {
    digitalWrite(RedLED, HIGH);
    delay(1000);
    digitalWrite(RedLED, LOW);
    delay(1000);
    x = x + 1; 
  } 
}


//Task2code: blinks an LED every 500 ms
void Task2code( void * pvParameters ){
  while(1)
  {
    digitalWrite(GreenLED, HIGH);
    delay(500);
    digitalWrite(GreenLED, LOW);
    delay(500);
    Serial.print(x);
  }
}



//Task1code: blinks an LED every 700 ms
void Task3code( void * pvParameters ){
  while(1)
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
      // TODO: turn off alarm, light or deactivate a device ... here
    }
    delay(1000);
  }
}


void loop() {
  
}
