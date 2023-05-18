

// #include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

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
#include "camMio.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

String chatId = String(CHAT_ID);
String chatId2 = String(CHAT_ID_2);
String BOTtoken = String(BOTtokenKey);
String sendPhotoReqID = String(CHAT_ID);
bool sendPhoto = false;


#define FEED_COUNT 2
#define SERVO_PIN 13
#define MOVE_PIN GPIO_NUM_14


int FeedCat = 0;
int FeedCatKp = 0;
int CountFeed = 0;

Servo myservo;  // create servo object to control a servo


bool flashState = LOW;

// Motion Sensor
bool motionDetected = false;
bool shutUp = true;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);
//Stores the camera configuration parameters
camera_config_t config;


// variabili relative ai task
long lastTimeBotRan;     // last time messages' scan has been done
unsigned long PreviousCount1s = 0;
unsigned long PreviousCount1m = 0;


void handleNewMessages(int numNewMessages);
String sendPhotoTelegram();


// Indicates when motion is detected
static void IRAM_ATTR detectsMovement(void * arg){
  
  if (!shutUp){
    #ifdef DEBUG_FEEDER
      Serial.println("MOTION DETECTED!!!");
    #endif
    motionDetected = true;    
  }
  else {
    #ifdef DEBUG_FEEDER
      Serial.println("MOTION DETECTED!!!, but disabled");
    #endif
  }
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  #ifdef DEBUG_FEEDER
    Serial.begin(115200);
  #endif
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);
  
  initWiFi_PwrOn(SSID, PASSWORD);
  

  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  configInitCamera_PwrOn();
  ServoFeed_PwrOn();


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

void loop(){

  if (sendPhoto){
    #ifdef DEBUG_FEEDER
      Serial.println("Preparing photo");
    #endif
    sendPhotoTelegram(); 
    sendPhoto = false; 
  }
  if (millis() > 1000 + PreviousCount1s)// TASK 1s
  {
    ServoFeed_tsk1S(&FeedCat);    
    PreviousCount1s = millis();
  }

  if (millis() > 60000 + PreviousCount1m) // TASK 6m
  {
  // questo voglio che venga fatto girare ad ogni secondo, forse meno
    if(motionDetected){
      #ifdef DEBUG_FEEDER
        Serial.println("Motion Detected");
      #endif
      bot.sendMessage(chatId, "Motion detected!!", "");
      bot.sendMessage(chatId2, "Motion detected!!", "");
      Serial.println("Motion Detected");
      sendPhoto = true; 
    }
    PreviousCount1m = millis();
  }


  if (millis() >  1000 + lastTimeBotRan) // TASK bot
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      #ifdef DEBUG_FEEDER
        Serial.println("got response");
      #endif
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

// in questa parte ci vado a mettere le funzioni

void ServoFeed_tsk1S(int *FeedCat)
{
    if (FeedCatKp == 1 && *FeedCat == 0) {
      FeedCatKp = 0;
      myservo.write(100);
      #ifdef DEBUG_FEEDER
        Serial.print("Debug State = ");
        Serial.println("3");
      #endif
    }  

    if (*FeedCat == 1) {
      FeedCatKp = 1;
      myservo.write(90);
      CountFeed = CountFeed + 1;

      #ifdef DEBUG_FEEDER
        Serial.print("Debug State = ");
        Serial.println("1");
      #endif

      if (CountFeed > FEED_COUNT){
        *FeedCat = 0;
        CountFeed = 0;
        #ifdef DEBUG_FEEDER
            Serial.print("Debug State = ");
            Serial.println("2");
        #endif
      }
    }
}

String sendPhotoTelegram(){
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get(); 
  if(!fb) {
    #ifdef DEBUG_FEEDER
      Serial.println("Camera capture failed");
    #endif
    delay(500);
    ESP.restart();
    return "Camera capture failed";
  }  
  #ifdef DEBUG_FEEDER
    Serial.println("Connect to " + String(myDomain));
  #endif
  if (clientTCP.connect(myDomain, 443)) {
    #ifdef DEBUG_FEEDER
      Serial.println("Connection successful");
    #endif
    String head = "--eof\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + sendPhotoReqID + "\r\n--eof\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--eof--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
    #ifdef DEBUG_FEEDER
      Serial.print("the length of the immage is: ");
      Serial.println(imageLen);
    #endif
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=eof");
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
      #ifdef DEBUG_FEEDER
        Serial.print(".");
      #endif
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
    #ifdef DEBUG_FEEDER
      Serial.println(getBody);
    #endif
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    #ifdef DEBUG_FEEDER
      Serial.println("Connected to api.telegram.org failed.");
    #endif
  }
  return getBody;
}

void handleNewMessages(int numNewMessages){
  #ifdef DEBUG_FEEDER
    Serial.print("Handle New Messages: ");
    Serial.println(numNewMessages);
  #endif

  for (int i = 0; i < numNewMessages; i++){
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if ((chat_id != chatId) and (chat_id != chatId2)){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    #ifdef DEBUG_FEEDER
      Serial.println(text);
    #endif
    String fromName = bot.messages[i].from_name;

    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
    }
    if (text == "/photo") {
      sendPhoto = true;
      sendPhotoReqID = String(bot.messages[i].chat_id);
      #ifdef DEBUG_FEEDER
        Serial.println("New photo  request");
      #endif
    }
    if (text == "/feed") {
      FeedCat = true;
      bot.sendMessage(chat_id, "I will feed the cat.\n", "Markdown");
    }
    if (text == "/shutup") {
      bot.sendMessage(chat_id, "see you next time.\n", "Markdown");
    }
    if (text == "/dontshutup") {
      bot.sendMessage(chat_id, "I here agian.\n", "Markdown");
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

// funzioni power on
void ServoFeed_PwrOn(void)
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
}


void configInitCamera_PwrOn(){
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 24000000;
  config.pixel_format = PIXFORMAT_JPEG; //YUV422,GRAYSCALE,RGB565,JPEG
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.frame_size = FRAMESIZE_HD; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10; //10-63 lower number means higher quality
  config.fb_count = 4;
  
  // Initialize the Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    #ifdef DEBUG_FEEDER
      Serial.printf("Camera init failed with error 0x%x", err);
    #endif
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
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


void initWiFi_PwrOn(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Attesa della connessione
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    #ifdef DEBUG_FEEDER
      Serial.println("Connessione in corso...");
    #endif
  }
  #ifdef DEBUG_FEEDER
    Serial.println("Connesso a " + String(SSID));
  #endif
}
