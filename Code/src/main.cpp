/*********
  Complete project details at https://randomnerdtutorials.com  
*********/


#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <Wire.h>
#include <SPI.h>



#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>



// MY LIBRERIES
#include <Feed.h>
#include <pass.h>



#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
unsigned long lastTimeInp = millis();

const int ledPin = 2;
bool ledState;

int pirPin = 16; // Input for HC-S501
int pirStateK; 
int pirStateKp; 

// Creazione dei tasks
unsigned long Count500ms;
unsigned long PreviousCount500ms = 0;

unsigned long Count1s;
unsigned long PreviousCount1s = 0;
//unsigned long lastTimeInp = millis();

int FeedCat = 0;



// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led_on to turn GPIO ON \n";
      welcome += "/led_off to turn GPIO OFF \n";
      welcome += "/humidity give the humidity\n";
      welcome += "/temperature give the temperature\n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    if (text == "/Feed") {      
      bot.sendMessage(chat_id, "I will Feed the cat", "");
      // TODO: andrebbe messa una variabile globale, in modo da non bloccare l'esecuzione del programma per 1 secondo qui
      FeedCat = 1;
    }
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}





void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();


    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("CatF_Wifi","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  #ifdef ESP32
  Serial.println("Sono passato da qui");
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif 

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  pinMode(pirPin, INPUT);

  ServoFeed_PwrOn();
}

void loop() {


  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }


  Count500ms = millis();
  if (Count500ms - PreviousCount500ms > 500)
  { 
    PreviousCount500ms = Count500ms;  

    pirStateK = digitalRead(pirPin);
    if (pirStateKp != pirStateK && pirStateK == 1){
      digitalWrite(ledPin, pirStateK);
      bot.sendMessage(CHAT_ID, "Motion Detected", "");
      Serial.println("Motion Detected");
    }

    pirStateKp = pirStateK;
  }

  Count1s = millis();
  if (Count1s - PreviousCount1s > 1000)
  { 
    PreviousCount1s = Count1s;

    ServoFeed_tsk1S(&FeedCat);
  }
}