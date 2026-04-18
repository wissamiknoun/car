#include <Arduino.h>
#include <ESP32Servo.h>
#include <BluetoothSerial.h>
#include "esp_bt.h"

#define REMOTEXY_MODE__ESP32CORE_BLUETOOTH
#define REMOTEXY_BLUETOOTH_NAME "ESP32_car"
#define REMOTEXY_ACCESS_PASSWORD ""

#include <RemoteXY.h>

/* ============================================================
   RemoteXY GUI Configuration (Classic BT, 38 bytes V19)
   ============================================================ */
#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =
{ 255,2,0,0,0,31,0,19,0,0,0,69,83,80,51,50,95,99,97,114,
  0,254,1,106,200,1,1,1,0,5,32,127,43,43,32,225,200,222 };

struct {
  int8_t  joystick_01_x;  // -100 to 100  (left/right → servo)
  int8_t  joystick_01_y;  // -100 to 100  (forward/back → motors)
  uint8_t connect_flag;   //  1 = connected
} RemoteXY;
#pragma pack(pop)

/* ============================================================
   Pin & Hardware Configuration
   ============================================================ */

// --- Motors (L298N / L293D style driver) ---
struct Motor {
  int en;          // Enable pin  (PWM)
  int in1;         // Direction pin 1
  int in2;         // Direction pin 2
  int pwmChannel;  // LEDC channel
};

Motor moteurs[2] = {
  {32, 33, 25, 1},   // Motor A →  EN=4  IN1=17  IN2=16  CH=1
  {14, 26, 27, 0}   // Motor B  →  EN=14  IN1=26  IN2=27  CH=0
};

// --- Servo ---
Servo myServo;
const int SERVO_PIN    = 13;
const int SERVO_CENTER = 90;
const int SERVO_MIN    =  0;
const int SERVO_MAX    = 180;

// --- Joystick dead zone ---
const int DEAD_ZONE = 10;

/* ============================================================
   Motor Helpers
   ============================================================ */
void setupMotor(Motor m) {
  pinMode(m.in1, OUTPUT);
  pinMode(m.in2, OUTPUT);
  ledcSetup(m.pwmChannel, 1000, 8);   // 1 kHz, 8-bit (0–255)
  ledcAttachPin(m.en, m.pwmChannel);
  ledcWrite(m.pwmChannel, 0);
}

void driveMotor(Motor m, bool forward, int speed) {
  digitalWrite(m.in1, forward ? HIGH : LOW);
  digitalWrite(m.in2, forward ? LOW  : HIGH);
  ledcWrite(m.pwmChannel, constrain(speed, 0, 255));
}

void stopCar() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(moteurs[i].in1, LOW);
    digitalWrite(moteurs[i].in2, LOW);
    ledcWrite(moteurs[i].pwmChannel, 0);
  }
}

/* ============================================================
   RemoteXY Event Callbacks
   ============================================================ */

// X axis → servo steering
void RemoteXY_joystick_01_x_event() {
  int angle = map(RemoteXY.joystick_01_x, -100, 100, SERVO_MIN, SERVO_MAX);
  myServo.write(angle);
  Serial.print("[SERVO] angle = ");
  Serial.println(angle);
}

// Y axis → motor drive
void RemoteXY_joystick_01_y_event() {
  int y     = RemoteXY.joystick_01_y;
  int speed = map(abs(y), 0, 100, 0, 255);

  if (y > DEAD_ZONE) {
    for (int i = 0; i < 2; i++)
      driveMotor(moteurs[i], true, speed);
    Serial.print("[MOTOR] FORWARD  speed = ");
    Serial.println(speed);
  }
  else if (y < -DEAD_ZONE) {
    for (int i = 0; i < 2; i++)
      driveMotor(moteurs[i], false, speed);
    Serial.print("[MOTOR] BACKWARD speed = ");
    Serial.println(speed);
  }
  else {
    stopCar();
    Serial.println("[MOTOR] STOP (dead zone)");
  }
}

/* ============================================================
   Setup
   ============================================================ */
void setup() {
  Serial.begin(115200);
  Serial.println("=== ESP32 Classic BT Car booting... ===");

  // 1. Claim LEDC channels for motors BEFORE BT stack starts
  for (int i = 0; i < 2; i++)
    setupMotor(moteurs[i]);
  Serial.println("[INIT] Motors ready");

  // 2. Attach servo (ESP32Servo auto-picks a free LEDC channel)
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_CENTER);
  Serial.println("[INIT] Servo ready (centered)");

  // 3. Start RemoteXY Classic BT stack
  RemoteXY_Init();
  esp_bredr_tx_power_set(ESP_PWR_LVL_P9, ESP_PWR_LVL_P9);
  Serial.println("[INIT] RemoteXY Classic BT started");

  // 4. Register event callbacks
  RemoteXYEngine.addVariableEvent(RemoteXY.joystick_01_x,
                                  RemoteXY_joystick_01_x_event);
  RemoteXYEngine.addVariableEvent(RemoteXY.joystick_01_y,
                                  RemoteXY_joystick_01_y_event);

  Serial.println("[INIT] Events registered");
  Serial.println("=== Ready – waiting for BT connection... ===");
}

/* ============================================================
   Loop  –  kept minimal on purpose
   ============================================================ */
void loop() {
  RemoteXYEngine.handler();   // drives BT comms + fires events

  // Detect connection state changes
  static uint8_t lastFlag = 255;
  if (RemoteXY.connect_flag != lastFlag) {
    lastFlag = RemoteXY.connect_flag;
    if (RemoteXY.connect_flag) {
      Serial.println("[BT] App connected");
      myServo.write(SERVO_CENTER);   // re-center on fresh connect
    } else {
      Serial.println("[BT] App disconnected – stopping car");
      stopCar();
      myServo.write(SERVO_CENTER);
    }
  }

  // NOTE: never use delay() here.
  // If you need a timed action use: RemoteXYEngine.delay(ms);
}