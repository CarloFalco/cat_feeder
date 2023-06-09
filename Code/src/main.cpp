#define DEBUG_FEEDER
#define DEBUG_Startup


#include <Wire.h>
#include <SPI.h>


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h> //

#include <ESP32Servo.h>// lireria per il controllo del servomotore

#include "passMio.h"
#include <MyFcn.h>
#include <Feed.h>


#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"


// Definisco le classi
TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;
Servo myServo =  //TODO

void setup() {
  // Inizializza il serial monitor
  #ifdef DEBUG_FEEDER
    Serial.begin(115200);
    while (!Serial);
  #endif
  ServoFeed_PwrOn();
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Led_PwrOn();




  IR_PwrOn();
  // Crea i task
  Scheduler_PwrOn();














  
  


  initWiFi_PwrOn(SSID, PASSWORD);
  

  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  configInitCamera_PwrOn();
  


  // PIR Motion Sensor mode INPUT_PULLUP
  //err = gpio_install_isr_service(0); 
  esp_err_t err = gpio_isr_handler_add(MOVE_PIN, &detectsMovement, (void *) 13);  
  if (err != ESP_OK){
    #ifdef DEBUG_FEEDER
      Serial.printf("handler add failed with error 0x%x \r\n", err); 
    #endif
  }
  err = gpio_set_intr_type(MOVE_PIN, GPIO_INTR_POSEDGE);
  if (err != ESP_OK){
    #ifdef DEBUG_FEEDER
      Serial.printf("set intr type failed with error 0x%x \r\n", err);
    #endif
  }

  #ifdef DEBUG_Startup
  String welcome = "Welcome to the ESP32-CAM Telegram bot.\n";
  welcome += "/photo : takes a new photo\n";
  welcome += "/flash : toggle flash LED\n";
  welcome += "/feed : feed the cat\n";
  welcome += "/shutup : disalbe motion sensor notification\n";
  welcome += "/dontshutup : enable motion sensor notification\n";
  welcome += "You'll receive a photo whenever motion is detected.\n";
  bot.sendMessage(chatId, welcome, "Markdown");
  #endif







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
    IR_Tsk(pinState);

    vTaskDelayUntil(&lastWakeTime, period);
  }
}


void loop() {
  // Il loop principale non fa nulla
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