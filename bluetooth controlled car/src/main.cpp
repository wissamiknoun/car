#include <Arduino.h>
#include <ESP32Servo.h>

#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "ESP32-Car"
#define REMOTEXY_ACCESS_PASSWORD ""

#include <BLEDevice.h>
#include <RemoteXY.h>

/* -------- Interface RemoteXY -------- */
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // config joystick simple
{ 255,1,0,0,0,17,0,8,13,0,
  5,0,10,10,80,80,1,26,31 };

struct {
  int8_t joystick_1_x; // -100 → 100
  int8_t joystick_1_y;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

/* -------- MOTEURS -------- */
struct Motor {
  int en, in1, in2, pwmChannel;
};

Motor moteurs[2] = {
  {16, 5, 17, 1},   // gauche
  {21, 19, 18, 2}   // droit
};

Servo myservo;

/* -------- SETUP MOTEUR -------- */
void setupMotor(Motor m) {
  pinMode(m.in1, OUTPUT);
  pinMode(m.in2, OUTPUT);

  ledcAttachPin(m.en, m.pwmChannel);
  ledcSetup(m.pwmChannel, 1000, 8);
  ledcWrite(m.pwmChannel, 0);
}

void driveMotor(Motor m, bool forward, int speed) {
  digitalWrite(m.in1, forward ? HIGH : LOW);
  digitalWrite(m.in2, forward ? LOW : HIGH);
  ledcWrite(m.pwmChannel, speed);
}

void stopCar() {
  for (int i=0;i<2;i++) {
    digitalWrite(moteurs[i].in1, LOW);
    digitalWrite(moteurs[i].in2, LOW);
    ledcWrite(moteurs[i].pwmChannel, 0);
  }
}

/* -------- SETUP -------- */
void setup() {
  Serial.begin(115200);

  RemoteXY_Init();

  myservo.attach(33);

  for (int i=0;i<2;i++)
    setupMotor(moteurs[i]);
}

/* -------- LOOP -------- */
void loop() {

  RemoteXY_Handler();

  int x = RemoteXY.joystick_1_x;
  int y = RemoteXY.joystick_1_y;

  // ===== SERVO DIRECTION =====
  int angle = map(x, -100, 100, 0, 180);
  myservo.write(angle);

  // ===== VITESSE =====
  int speed = map(abs(y), 0, 100, 0, 255);

  if (y > 10) {
    for (int i=0;i<2;i++)
      driveMotor(moteurs[i], true, speed);
  }
  else if (y < -10) {
    for (int i=0;i<2;i++)
      driveMotor(moteurs[i], false, speed);
  }
  else {
    stopCar();
  }
}
