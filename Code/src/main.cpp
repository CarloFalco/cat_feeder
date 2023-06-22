#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;

//  pins
const int RedLED = 4;
const int GreenLED = 14;

int x = 0;

int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

void Scheduler_PwrOn(void);
void Led_PwrOn(void);

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
  const TickType_t period = pdMS_TO_TICKS(200);  // 500 millisecondi

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
    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void setup() {
  // Inizializza il serial monitor
  Serial.begin(115200);
  while (!Serial);

  Led_PwrOn();

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