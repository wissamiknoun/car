#include <Arduino.h>
#include <BluetoothSerial.h>

// Vérifier si Bluetooth est activé
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig`
#endif

BluetoothSerial SerialBT;

// =====================
// === CONFIG MOTEURS ===
// =====================

// Driver 1
int enA1 = 14;
int in11 = 27;
int in21 = 26;

int in31 = 25;
int in41 = 33;
int enB1 = 32;

// Driver 2
int enA2 = 21;
int in12 = 19;
int in22 = 18;

int in32 = 5;
int in42 = 17;
int enB2 = 16;

// =====================
// === PARAMÈTRES ===
// =====================
bool turboMode = false;
int vitesseNormale = 180;
int vitesseTurbo = 255;

// Variables pour stocker les données de la manette
int joystickLX = 0;
int joystickLY = 0;
bool buttonA = false;

// =====================
// === FONCTIONS MOTEURS
// =====================
void setMotor(int in1, int in2, int channel, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(channel, speed);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(channel, abs(speed));
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(channel, 0);
  }
}

void setLeftMotors(int speed) {
  setMotor(in11, in21, 1, speed);
  setMotor(in32, in42, 4, speed);
}

void setRightMotors(int speed) {
  setMotor(in31, in41, 2, speed);
  setMotor(in12, in22, 3, speed);
}

void stopCar() {
  setLeftMotors(0);
  setRightMotors(0);
}

// =====================
// === SETUP ===
// =====================
void setup() {
  Serial.begin(115200);

  // Démarrer Bluetooth Serial
  SerialBT.begin("ESP32_RC_Car"); 
  Serial.println("🎮 Bluetooth démarré ! Nom : ESP32_RC_Car");
  Serial.println("📱 Utilisez une application comme 'Bluetooth Controller' sur smartphone");

  // Pins
  pinMode(in11, OUTPUT); pinMode(in21, OUTPUT);
  pinMode(in31, OUTPUT); pinMode(in41, OUTPUT);
  pinMode(in12, OUTPUT); pinMode(in22, OUTPUT);
  pinMode(in32, OUTPUT); pinMode(in42, OUTPUT);

  // PWM Setup
  ledcSetup(1, 1000, 8);
  ledcSetup(2, 1000, 8);
  ledcSetup(3, 1000, 8);
  ledcSetup(4, 1000, 8);

  ledcAttachPin(enA1, 1);
  ledcAttachPin(enB1, 2);
  ledcAttachPin(enA2, 3);
  ledcAttachPin(enB2, 4);

  stopCar();
}

// =====================
// === LOOP ===
// =====================
void loop() {
  // Lire les commandes depuis Bluetooth
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    data.trim();
    
    // Format attendu : "LX:123,LY:456,A:1"
    if (data.length() > 0) {
      // Parser les données
      int lxIndex = data.indexOf("LX:");
      int lyIndex = data.indexOf("LY:");
      int aIndex = data.indexOf("A:");
      
      if (lxIndex != -1) {
        joystickLX = data.substring(lxIndex + 3, data.indexOf(',', lxIndex)).toInt();
      }
      if (lyIndex != -1) {
        joystickLY = data.substring(lyIndex + 3, data.indexOf(',', lyIndex)).toInt();
      }
      if (aIndex != -1) {
        buttonA = data.substring(aIndex + 2).toInt() == 1;
      }
    }
  }

  // Contrôle des moteurs
  turboMode = buttonA;
  int maxSpeed = turboMode ? vitesseTurbo : vitesseNormale;

  int ly = joystickLY;
  int lx = joystickLX;

  // Dead Zone
  if (abs(ly) < 40) ly = 0;
  if (abs(lx) < 40) lx = 0;

  // Mixage différentiel
  int left = ly + lx;
  int right = ly - lx;

  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  // Limiter à la vitesse max
  if (left > 0) left = map(left, 0, 255, 0, maxSpeed);
  else left = map(left, -255, 0, -maxSpeed, 0);
  
  if (right > 0) right = map(right, 0, 255, 0, maxSpeed);
  else right = map(right, -255, 0, -maxSpeed, 0);

  setLeftMotors(left);
  setRightMotors(right);

  delay(10);
}