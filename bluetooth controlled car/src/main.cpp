#include <Arduino.h>
#include "BluetoothSerial.h"
#include <ESP32Servo.h>

BluetoothSerial SerialBT;

// Moteurs
struct Motor {
  int en, in1, in2, pwmChannel;
};

Motor moteurs[2] = {
  {16, 5, 17, 1},   // moteur gauche
  {21, 19, 18, 2}   // moteur droit
};

// vitesses
int x, y;
bool turboMode = false;
int vitesseNormale = map(abs(y), 0, 100, 0, 125);
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
  for (int i=0; i<2; i++) {
    digitalWrite(moteurs[i].in1, LOW);
    digitalWrite(moteurs[i].in2, LOW);
    ledcWrite(moteurs[i].pwmChannel, 0);
  }
}

int getSpeed() {
  return turboMode ? vitesseTurbo : vitesseNormale;
}

// Fonctions déplacement
void avancer() {
  for (int i=0; i<2; i++) driveMotor(moteurs[i], true, getSpeed());
}

void reculer() {
  for (int i=0; i<2; i++) driveMotor(moteurs[i], false, getSpeed());
}

//servomotor

Servo myservo;  // create Servo object to control a servo

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-Car");
  Serial.println("Bluetooth prêt !");

  myservo.attach(33);
  
  for (int i=0; i<2; i++) setupMotor(moteurs[i]);
}

void loop() {
  // ===== BLUETOOTH COMMANDES =====
  while (SerialBT.available()) {

    char c = SerialBT.read();

    if (c == '\n' || c == '\r') {

      cmd.trim();
      Serial.println("Commande reçue : " + cmd);

      if (cmd.startsWith("J")) {
        
        sscanf(cmd.c_str(), "J,%d,%d", &x, &y);

        //direction
        int angle = map(x, -100, 100, 0, 180);
        myservo.write(angle);
        Serial.println(angle);

        //vitesse
        if (y > 10) {
          avancer();
        }
        else if (y < -10) {
          reculer();
        }
        else {
          stopCar();
        }
      }
      cmd = "";
    }
    else {
      cmd += c;
    }
  }
  delay(10);
}
