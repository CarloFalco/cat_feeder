TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;

//  pins
const int RedLED = 12;
const int GreenLED = 14;
const int PIN_TO_SENSOR = 34;

int x = 0;

int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin


void task1(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);  // 1 secondo

  while (true) {
    // Esegui il codice del task 1
      digitalWrite(RedLED, !digitalRead(RedLED));
      x = x + 1;

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task2(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(500);  // 500 millisecondi

  while (true) {
    // Esegui il codice del task 2
      digitalWrite(GreenLED, !digitalRead(GreenLED));
      Serial.print(x);

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task3(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(200);  // 200 millisecondi

  while (true) {
    // Esegui il codice del task 3
    IR_Tsk();

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void setup() {
  // Inizializza il serial monitor
  Serial.begin(115200);
  while (!Serial);

  Led_PwrOn();
  IR_PwrOn();

  // Crea i task
  Scheduler_PwrOn();
}

void loop() {
  // Il loop principale non fa nulla

}


// CONDIZIONI INIZIALI//

void Led_PwrOn(void)
{
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
}

void IR_Tsk(void)
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

void Scheduler_PwrOn(void)
{
  xTaskCreatePinnedToCore(
                    task1,   // Task function.
                    "Task1",     // name of task.
                    2048,       // Stack size of task
                    NULL,        // parameter of the task
                    1,           // priority of the task 
                    &task1Handle,      // Task handle to keep track of created task
                    1);          // pin task to core 0                 
  
  xTaskCreatePinnedToCore(
                    task2,   // Task function.
                    "Task2",     // name of task. 
                    2048,       // Stack size of task 
                    NULL,        // parameter of the task 
                    2,           // priority of the task 
                    &task2Handle,      // Task handle to keep track of created task 
                    1);          // pin task to core 1

  xTaskCreatePinnedToCore(
                    task3,   // Task function.
                    "Task3",     // name of task. 
                    2048,       // Stack size of task 
                    NULL,        // parameter of the task 
                    3,           // priority of the task 
                    &task3Handle,      // Task handle to keep track of created task 
                    1);          // pin task to core 1
}