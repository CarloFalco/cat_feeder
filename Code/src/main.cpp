#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "passMio.h"
#include "MyFcn.h"


TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;

//  pins

void Scheduler_PwrOn(void);

// DEBUG stuff
#define DEBUG_SERVO




////////////////////////////////////////////////////////////////

void setup() {
  // Inizializza il serial monitor
  Serial.begin(115200);
  Serial.println("********************************");
  Serial.println("Hello, world!");
  //initWiFi_PwrOn(NETWORK_SSID, PASSWORD);


  //Led_PwrOn();
  //ServoFeed_PwrOn();


  // Crea i task
  Scheduler_PwrOn();
}


void task1(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);  // 1 secondo

  while (true) {
    // Esegui il codice del task 1





    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task2(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(500);  // 500 millisecondi

  while (true) {
    // Esegui il codice del task 2




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

////////////////////////////////////////////////////////////////////////

void loop() {
  // Il loop principale non fa nulla
  Serial.println("Hello, world!");
  #ifdef DEBUG_SERVO
    if (Serial.available() > 0)
    {
      int number = Serial.parseInt();
      Serial.println(number);
      ServoFeed_Tsk(number);
    }
  #endif
  delay(1000);
  
}


// CONDIZIONI INIZIALI//
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
                    1,           // priority of the task 
                    &task2Handle,      // Task handle to keep track of created task 
                    1);          // pin task to core 1

  xTaskCreatePinnedToCore(
                    task3,   // Task function.
                    "Task3",     // name of task. 
                    2048,       // Stack size of task 
                    NULL,        // parameter of the task 
                    1,           // priority of the task 
                    &task3Handle,      // Task handle to keep track of created task 
                    1);          // pin task to core 1
}