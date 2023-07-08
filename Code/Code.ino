#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ESP32Servo.h>
// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you

#include "Credential.h"

// FLASH
#define FLASH_LED_PIN 4
// SERVOMOTORE
#define FEED_COUNT 2
#define SERVO_PIN 13
#define SERVO_MOVE_ON 95
#define SERVO_MOVE_OFF 90
// SENSORE IR
#define IR_PIN 34

// DEFINE FOR DEBUG
#define DEBUG_WIFI
#define DEBUG_FEEDER



TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;

typedef struct {
  // in case of fail std::string name1[4];
  String CurrentPhotoReqId;      // ID del utente che ha richiesto la foto
  String AuthId[4];         // ID degli utenti
  bool NotificationId[4];   // Flag di abilitazione notifiche utenti [1/0] abilitato/non abilitato
  bool stateIRpin;          // stato del sensore infrarossi
  bool stateSendPhoto;      // stato richiesta di foto
  bool stateFeedCat;        // stato richiesta feedcat
  bool stateFlash;          // stato flash
} MyStruct;

MyStruct myStruct = {
                       String(""),
                       {String(CHAT_ID_1), String(CHAT_ID_2), String(""), String("")},
                       {1, 1, 1, 1},
                       false, //stateIRpin
                       false, //stateSendPhoto
                       false, //stateFeedCat
                       LOW  // stato flash
                       };



WiFiClientSecure clientTCP;

String BOTtoken = String(BOTtokenKey);
UniversalTelegramBot bot(BOTtoken, clientTCP);

Servo myservo;  // create servo object to control a servo

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void initWiFi_PwrOn(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  // Attesa della connessione
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    #ifdef DEBUG_WIFI
      Serial.println("Connessione in corso...");
    #endif
  }
  
  #ifdef DEBUG_WIFI
    Serial.println("WiFi connected to " + String(ssid));
    Serial.println(WiFi.localIP());
  #endif

}

void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 24000000;
  config.pixel_format = PIXFORMAT_JPEG; //YUV422,GRAYSCALE,RGB565,JPEG
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.frame_size = FRAMESIZE_HD; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10; //10-63 lower number means higher quality
  config.fb_count = 4;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_HD);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

}


// FUNZIONI RELATIVE AL SERVO MOTORE
void ServoFeed_PwrOn(void){
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object

  myservo.write(SERVO_MOVE_OFF);
}

void ServoFeed_tsk1S(void){
  static int CountFeed;

  Serial.print(CountFeed);
  Serial.print("<- Valore in ingresso");

  if (myStruct.stateFeedCat == 1) {
    myservo.write(SERVO_MOVE_ON);
    CountFeed = CountFeed + 1;
    #ifdef DEBUG_FEEDER
      Serial.print("Debug State = ");
      Serial.println("1");
    #endif
  }
    if (CountFeed > FEED_COUNT){
      myservo.write(SERVO_MOVE_OFF);
      myStruct.stateFeedCat = 0;
      CountFeed = 0;
      #ifdef DEBUG_FEEDER
          Serial.print("Debug State = ");
          Serial.println("2");
      #endif
    }
  
}



// FUNZIONI RELATIVE AL SENSORE IR
void IR_PwrOn(void){
  // definizione tipologia pin
  pinMode(IR_PIN, INPUT);
  // condizione iniziale IR
  myStruct.stateIRpin = digitalRead(IR_PIN);
}

void IR_Tsk(void) {
  static bool pinState_kp; 
  static bool pinState; 
  pinState = digitalRead(IR_PIN);   // read new state

  if (pinState_kp == LOW && pinState == HIGH) {   // pin state change: LOW -> HIGH
    Serial.println("Motion detected!");
    myStruct.stateIRpin = true; // Comunico che ho visto un movimento (poi posso mandare la foto)
  }
  else
  if (pinState_kp == HIGH && pinState == LOW) {   // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
  }
  pinState_kp = pinState; // store old state
}



void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);


    // verifico che sia un utente autorizzato
    int checkIdAuth = 0;
    int j = 0;
    for (j = 0; j < sizeof(myStruct.AuthId); j++){ // TODO
      if (chat_id == myStruct.AuthId[j])
      {
        checkIdAuth = 1;
        break;
      }
    }
      if (!checkIdAuth){
        bot.sendMessage(chat_id, "Unauthorized user", "");
        continue;
      }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;

    if (text == "/feed") {
      myStruct.stateFeedCat = true;
      bot.sendMessage(chat_id, "I will feed the cat.\n", "Markdown");
    }

    if (text == "/flash") {
      myStruct.stateFlash = !myStruct.stateFlash;
      digitalWrite(FLASH_LED_PIN, myStruct.stateFlash);
      Serial.println("Change flash LED state");
    }
    if (text == "/photo") {
      myStruct.stateSendPhoto = true;
      myStruct.CurrentPhotoReqId = chat_id;
      Serial.println("New photo request");
    }
    if (text == "/start"){
      String welcome = "Welcome to the ESP32-CAM Telegram bot.\n";
      welcome += "/photo : takes a new photo\n";
      welcome += "/flash : toggle flash LED\n";
      welcome += "/feed : feed the cat\n";
      welcome += "/shutup : disalbe motion sensor notification\n";
      welcome += "/dontshutup : enable motion sensor notification\n";
      welcome += "You'll receive a photo whenever motion is detected.\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }

  }
}

String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--EOF\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + myStruct.CurrentPhotoReqId + "\r\n--EOF\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--EOF--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=EOF");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void Led_PwrOn(void){
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, myStruct.stateFlash);
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  Serial.begin(115200);

  // Set LED Flash as output
  Led_PwrOn();

  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  initWiFi_PwrOn(NETWORK_SSID, PASSWORD);

  // Configure Servo Motor
  ServoFeed_PwrOn();

  // Crea i task
  Scheduler_PwrOn();
}

void task1(void* pvParameters) {// 1 secondo
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);  

  while (true) {
    // Esegui il codice del task 1
    //Serial.println("Task 1");


    vTaskDelayUntil(&lastWakeTime, period);
  }
}


void task2(void* pvParameters) {// 500 millisecondi
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(500);  

  while (true) {
    // Esegui il codice del task 2

    //Serial.println("Task 2");
    if (myStruct.stateFeedCat)
    {
      ServoFeed_tsk1S();
    }

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void task3(void* pvParameters) {// 200 millisecondi
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(200);  

  while (true) {
    // Esegui il codice del task 3
    //Serial.println("Task 3");

    vTaskDelayUntil(&lastWakeTime, period);
  }
}



void loop() {

    if (myStruct.stateSendPhoto) {
      Serial.println("Preparing photo");
      sendPhotoTelegram(); 
      myStruct.stateSendPhoto = false; 
    }

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}



// CONDIZIONI INIZIALI//
void Scheduler_PwrOn(void){
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
