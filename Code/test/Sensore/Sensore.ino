#include <AsyncDelay.h>

#include <Wire.h>
#include <SPI.h>
#include <ESP32Servo.h>// lireria per il controllo del servomotor

#define SERVO_PIN 13 // Pin del servomotore
#define FLASH_LED_PIN 4

// Dichiarazione delle costanti
const int pinSensoreIR = 14;  // Pin del sensore infrarossi


// Variabili per memorizzare lo stato del sensore e il valore letto dal monitor seriale
int statoSensoreIR = 0;
int valoreLetto = 0;

// Creazione di un oggetto di tipo AsyncDelay
AsyncDelay asyncDelay;
AsyncDelay asyncSerial;

// Dichiarazione delle funzioni dei task
void checkMovement();
void readSerial();

unsigned long PreviousCount1m = 0;

// Creazione di un oggetto di tipo Servo
Servo servoMotor;

void setup() {
  // Inizializzazione del monitor seriale
  Serial.begin(9600);

  // Inizializzazione del servomotore
  ServoFeed_PwrOn();
  
  // Imposta il pin del sensore infrarossi come input
  pinMode(pinSensoreIR, INPUT);
  
  // Imposta il pin del LED come output
  pinMode(FLASH_LED_PIN, OUTPUT);

  // Imposta il tempo di attesa per il task di checkMovement a 1000 millisecondi (1 secondo)
  asyncDelay.start(1000, AsyncDelay::MILLIS);
  
  // Imposta il tempo di attesa per il task di readSerial a 100 millisecondi (0.1 secondo)
  asyncSerial.start(100, AsyncDelay::MILLIS);


}

void loop() {
  if (asyncDelay.isExpired()) {
    checkMovement();
    asyncDelay.repeat(); // Count from when the delay expired, not now
  }

  if (asyncSerial.isExpired()) {
    readSerial();
    asyncSerial.repeat();
  }
}



void ServoFeed_PwrOn(void)
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servoMotor.setPeriodHertz(50);    // standard 50 hz servo
  servoMotor.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object

  servoMotor.write(90);
}


// Definizione della funzione del task per verificare il movimento
void checkMovement() {
  // Leggi lo stato del sensore infrarossi
  statoSensoreIR = digitalRead(pinSensoreIR);
  
  // Se viene rilevato movimento, accendi il LED
  if (statoSensoreIR == HIGH) { 
    digitalWrite(FLASH_LED_PIN, HIGH);  // Accendi il LED
    Serial.println("Movimento rilevato!");
    PreviousCount1m = millis();
  }
  else {
    digitalWrite(FLASH_LED_PIN, LOW);  // Spegni il LED
  }
}

// Definizione della funzione del task per leggere il monitor seriale
void readSerial() {
  // Se ci sono dati disponibili sul monitor seriale
  if (Serial.available()) {
    valoreLetto = Serial.parseInt();  // Leggi un numero intero dal monitor seriale
    
    // Verifica se il valore letto è valido (diverso da zero)
    if (valoreLetto != 0) {
      // Mappa il valore letto nell'intervallo di angoli del servomotore
      int angolo = map(valoreLetto, 0, 1023, 0, 180);
      
      // Muovi il servomotore all'angolo desiderato
      servoMotor.write(angolo);
    }
  }
}