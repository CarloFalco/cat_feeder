#ifndef MYFCN_H
#define MYFCN_H

#include "passMio.h"
#include <Arduino.h> 

#define FLASH_LED_PIN     4

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

// SERVOMOTORE
#define FEED_COUNT 2
#define SERVO_PIN 15
#define SERVO_MOVE_ON 90
#define SERVO_MOVE_OFF 95



typedef struct {
  // in case of fail std::string name1[4];
  String BOTtoken;          // ID del bot TELEGRAM
  String CurrentPhotoReqId;      // ID del utente che ha richiesto la foto
  String AuthId[4];         // ID degli utenti
  bool NotificationId[4];   // Flag di abilitazione notifiche utenti [1/0] abilitato/non abilitato
  bool stateIRpin;          // stato del sensore infrarossi
  bool stateSendPhoto;      // stato richiesta di foto
  bool stateFeedCat;        // stato richiesta feedcat
  bool stateFlash;          // stato flash
} MyStruct;

MyStruct myStruct = {
                      String(BOTtokenKey),
                      {String(CHAT_ID_1), String(CHAT_ID_2), String(""), String("")},
                      {1, 1, 1, 1}, 
                      String(""),
                      false, //stateIRpin
                      false, //stateSendPhoto
                      false, //stateFeedCat
                      LOW  // stato flash
                      };

// esempio di come si struttura una libreria

// funzioni di inizializzazione
void IR_PwrOn();
void initWiFi_PwrOn(const char* , const char* );
void configInitCamera_PwrOn();
void Led_PwrOn();
void ServoFeed_PwrOn(void);

// FUNZIONI nei task
void IR_Tsk(int& );
void handleNewMessages(int);
void ServoFeed_tsk1S(int *FeedCat);


#endif