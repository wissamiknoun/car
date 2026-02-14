#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Moteurs
struct Motor {
  int en, in1, in2, pwmChannel;
};

Motor moteurs[4] = {
  {14, 27, 26, 1},  // moteur 1
  {32, 25, 33, 2},  // moteur 2
  {16, 5, 17, 4},   // moteur 3
  {21, 19, 18, 3}   // moteur 4
};

// vitesses
bool turboMode = false;
int vitesseNormale = 125;
int vitesseTurbo = 255;

// Commande Bluetooth
String cmd = "";

// Setup moteurs
void setupMotor(Motor m) {
  pinMode(m.en, OUTPUT);
  pinMode(m.in1, OUTPUT);
  pinMode(m.in2, OUTPUT);
  ledcAttachPin(m.en, m.pwmChannel);
  ledcSetup(m.pwmChannel, 1000, 8);
  ledcWrite(m.pwmChannel, 0);
}

// Contrôle moteur
void driveMotor(Motor m, bool forward, int speed) {
  digitalWrite(m.in1, forward ? HIGH : LOW);
  digitalWrite(m.in2, forward ? LOW : HIGH);
  ledcWrite(m.pwmChannel, speed);
}

// Stop tous les moteurs
void stopCar() {
  for (int i=0; i<4; i++) {
    digitalWrite(moteurs[i].in1, LOW);
    digitalWrite(moteurs[i].in2, LOW);
    ledcWrite(moteurs[i].pwmChannel, 0);
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-Car");
  Serial.println("Bluetooth prêt !");
  
  for (int i=0; i<4; i++) setupMotor(moteurs[i]);
}

int getSpeed() {
  return turboMode ? vitesseTurbo : vitesseNormale;
}

// Fonctions déplacement
void avancer() {
  for (int i=0; i<4; i++) driveMotor(moteurs[i], true, getSpeed());
}

void reculer() {
  for (int i=0; i<4; i++) driveMotor(moteurs[i], false, getSpeed());
}

void gauche() {
  // gauche recule, droite avance
  driveMotor(moteurs[0], false, getSpeed());
  driveMotor(moteurs[2], false, getSpeed());
  driveMotor(moteurs[1], true, getSpeed());
  driveMotor(moteurs[3], true, getSpeed());
}

void droite() {
  driveMotor(moteurs[0], true, getSpeed());
  driveMotor(moteurs[2], true, getSpeed());
  driveMotor(moteurs[1], false, getSpeed());
  driveMotor(moteurs[3], false, getSpeed());
}

void loop() {
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      cmd.trim();
      Serial.println("Commande reçue : " + cmd);

      if (cmd == "avant") avancer();
      else if (cmd == "arriere") reculer();
      else if (cmd == "gauche") gauche();
      else if (cmd == "droite") droite();
      else if (cmd == "stop") stopCar();
      else if (cmd == "turbo_on") turboMode = true;
      else if (cmd == "turbo_off") turboMode = false;

      cmd = "";
    } else {
      cmd += c;
    }
  }
}
