#include <ESP32Servo.h>
#include <Feed.h>


int FeedCatKp = 0;
int CountFeed = 0;

Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32


void ServoFeed_PwrOn(void)
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
}

void ServoFeed_tsk1S(int *FeedCat)
{
    if (FeedCatKp == 1 && *FeedCat == 0) {
      FeedCatKp = 0;
      myservo.write(95);
      #ifdef DEBUG_FEEDER
        Serial.print("Debug State = ");
        Serial.println("3");
      #endif
    }  

    if (*FeedCat == 1) {
      FeedCatKp = 1;
      myservo.write(85);
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