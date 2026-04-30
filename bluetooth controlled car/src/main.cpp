#include <Arduino.h>
#include <ESP32Servo.h>
#include <BluetoothSerial.h>
#include "esp_bt.h"


#define REMOTEXY_MODE__ESP32CORE_BLUETOOTH
#define REMOTEXY_BLUETOOTH_NAME "ESP32-CAR"

#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 38 bytes V19 
  { 255,2,0,0,0,31,0,19,0,0,0,69,83,80,51,50,95,99,97,114,
  0,254,1,106,200,1,1,1,0,5,32,127,43,43,32,225,200,222 };

struct {
  int8_t  joystick_01_x;  // -100 to 100  (left/right → servo)
  int8_t  joystick_01_y;  // -100 to 100  (forward/back → motors)
  uint8_t connect_flag;   //  1 = connected
} RemoteXY;
#pragma pack(pop)

#include <RemoteXY.h>

BluetoothSerial SerialBT;

String cmd = "";

// Moteur 1
int enA = 21;
int in1 = 19;
int in2 = 18;

// Moteur 2
int in3 = 5;
int in4 = 17;
int enB = 16;

Servo myServo;
const int SERVO_PIN    = 13;
const int SERVO_CENTER = 90;
const int SERVO_MIN    = 120;
const int SERVO_MAX    = 60;

void movemotors(int x, int y) {

  if (y > 10) {
    // Avancer
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  } else if (y < -10) {
    // Reculer
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  } else {
    // Arrêt
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }

  int speed = map(abs(y), 0, 100, 0, 255);

  if (x==0){
    ledcWrite(1, speed);
    ledcWrite(2, speed);
  }else if (x > 0)
  {
    int speeddroite = map(abs(x), 0, 100, 255, 0);
    ledcWrite(1, 255);
    ledcWrite(2, speeddroite);
  }else{
    int speedgauche = map(abs(x), 0, 100, 255, 0);
    ledcWrite(1, speedgauche);
    ledcWrite(2, 255);
  }
} 

void moveservo(int x) {
  int angle = map(x, -100, 100, SERVO_MIN, SERVO_MAX);
  Serial.print("X: "); Serial.print(x);
  Serial.print(" → Angle: "); Serial.println(angle);
  myServo.write(angle);
}

void RemoteXY_joystick_01_y_event() {
  movemotors(RemoteXY.joystick_01_x, RemoteXY.joystick_01_y);
}

void RemoteXY_joystick_01_x_event() {
  moveservo(RemoteXY.joystick_01_x);
}

void setup() {
  Serial.begin(115200);
  RemoteXY_Init ();
  
  RemoteXYEngine.addVariableEvent (RemoteXY.joystick_01_x, RemoteXY_joystick_01_x_event);
  RemoteXYEngine.addVariableEvent (RemoteXY.joystick_01_y, RemoteXY_joystick_01_y_event);
  Serial.println("Bluetooth prêt !");

  // Broches moteur en sortie
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  // Initialement arrêtés
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  // configurer PWM
  ledcAttachPin(enA, 1); 
  ledcSetup(1, 1000, 8); // canal 1, 1kHz, 8-bit (virtual PMW generator)
  ledcWrite(1, 0);

  ledcAttachPin(enB, 2);
  ledcSetup(2, 1000, 8);
  ledcWrite(2, 0);

  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_CENTER);
  Serial.println(myServo.read());

}

void loop() {
  RemoteXYEngine.handler ();

  delay(50);
}
