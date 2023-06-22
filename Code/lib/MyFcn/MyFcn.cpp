#include "MyFcn.h"    
#include <ESP32Servo.h>

#include "Arduino.h"  


int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

int FeedCatKp = 0;
int CountFeed = 0;

const int PIN_TO_SENSOR = 34;

Servo myservo;  // create servo object to control a servo


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




void handleNewMessages(int numNewMessages){
  #ifdef DEBUG_FEEDER
    Serial.print("Handle New Messages: ");
    Serial.println(numNewMessages);
  #endif

  for (int i = 0; i < numNewMessages; i++){
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;

    // verifico che sia un utente autorizzato
    int checkIdAuth = 0;
    for (int j = 0; j < sizeof(myStruct.AuthId); j++){ // TODO
      if ((chat_id == myStruct.AuthId[j]){
        checkIdAuth = 1;
        break;
      }
    }
      if (!checkIdAuth){
        bot.sendMessage(chat_id, "Unauthorized user", "");
        continue;
      }
    

    if (text == "/flash") {
      myStruct.stateFlash = !myStruct.stateFlash; // TODO aggiungere notifica stato flash
      digitalWrite(FLASH_LED_PIN, myStruct.stateFlash);
    }

    if (text == "/photo") {
      myStruct.stateSendPhoto = true;
      myStruct.CurrentPhotoReqId = String(bot.messages[i].chat_id);
      #ifdef DEBUG_FEEDER
        Serial.println("New photo  request");
      #endif
    }

    if (text == "/feed") {
      myStruct.stateFeedCat = true;
      bot.sendMessage(chat_id, "I will feed the cat.\n", "Markdown");
    }

    if (text == "/shutup") {
      bot.sendMessage(chat_id, "see you next time.\n", "Markdown");
      myStruct.NotificationId[j] = 0;
    }

    if (text == "/dontshutup") {
      bot.sendMessage(chat_id, "I here agian.\n", "Markdown");
      myStruct.NotificationId[j] = 1;
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

void ServoFeed_tsk1S(int *FeedCat)
{
    if (FeedCatKp == 1 && *FeedCat == 0) {
      FeedCatKp = 0;
      myservo.write(SERVO_MOVE_ON);
      #ifdef DEBUG_FEEDER
        Serial.print("Debug State = ");
        Serial.println("3");
      #endif
    }  

    if (*FeedCat == 1) {
      FeedCatKp = 1;
      myservo.write(SERVO_MOVE_OFF);
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


// FUNZIONI DI INIZIALIZZAZIONE
// funzioni power on

void Led_PwrOn(void)
{
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, myStruct.stateFlash);
}

void ServoFeed_PwrOn(void)
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
}


void configInitCamera_PwrOn(void){
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


void ServoFeed_PwrOn(void)
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object

  myservo.write(90);
}


void IR_PwrOn(void)
{
  // definizione tipologia pin
  pinMode(PIN_TO_SENSOR, INPUT);
  // condizione iniziale IR
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);
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

