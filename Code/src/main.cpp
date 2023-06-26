#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "passMio.h"
#include "MyFcn.h"

#include <ESP32Servo.h>
#include <WiFi.h>



TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;
TaskHandle_t task4Handle;
//  pins

void Scheduler_PwrOn(void);


///////////////////////////////////////////////////////////////

void setup() {
  // Inizializza il serial monitor
  Serial.begin(115200);
  Serial.println("********************************");
  Serial.println("Hello, world!");
  initWiFi_PwrOn(NETWORK_SSID, PASSWORD);


  Led_PwrOn();
  ServoFeed_PwrOn();


  // Crea i task
  Scheduler_PwrOn();
}

/* void KeepWiFiAlive(void* pvParameters){// 10 secondo
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    { 
      #ifdef DEBUG_WIFI
        Serial.println("Wifi still connected");
      #endif
      vTaskDelay(10000/portTICK_PERIOD_MS);
      continue;  
    }
    #ifdef DEBUG_WIFI
      Serial.println("WiFi is connecting");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(NETWORK_SSID, PASSWORD);

    unsigned long startAttemptTime = millis();

    // Keep looping while we're not connected and haven't reached the timeout
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){}

    if (WiFi.status() != WL_CONNECTED){
      #ifdef DEBUG_WIFI
        Serial.println("WiFi Failed");
      #endif
      vTaskDelay(20000/portTICK_PERIOD_MS);
      continue;
    }
    #ifdef DEBUG_WIFI
      Serial.println("Wifi Connected: " + WiFi.localIP());
    #endif

  }
}
 */

void task1(void* pvParameters) {// 1 secondo
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);  

  while (true) {
    checkNewMessage();

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task2(void* pvParameters) {// 500 millisecondi
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(500);  

  while (true) {
    // Esegui il codice del task 2




    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task3(void* pvParameters) {// 200 millisecondi
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(200);  

  while (true) {
    // Esegui il codice del task 3






    vTaskDelayUntil(&lastWakeTime, period);
  }
}

////////////////////////////////////////////////////////////////////////

void loop() {
  // Il loop principale non fa nulla

  #ifdef DEBUG_SERVO
    if (Serial.available() > 0)
    {
      int number = Serial.parseInt();
      if (number>0)
        {
          ServoFeed_Tsk(number);
        }
    }
  #endif

  
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
/*                    
  xTaskCreatePinnedToCore(
                     KeepWiFiAlive,
                    "Keep WiFi Alive",     // name of task. 
                    2048,       // Stack size of task 
                    NULL,        // parameter of the task 
                    1,           // priority of the task 
                    &task4Handle,      // Task handle to keep track of created task 
                    1);          // pin task to core 1 
                    */
}