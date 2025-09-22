#include <Arduino.h>
#include "BluetoothSerial.h"


BluetoothSerial SerialBT;

String cmd = "";

// Moteur A 
int enA = 25;
int in1 = 5;
int in2 = 17;

// Vitesse normale et turbo
bool turboMode = false;
int vitesseNormale = 80;
int vitesseTurbo = 255;
int vitesse = vitesseNormale;

void setup() {
  
  Serial.begin(115200);          // Moniteur série
  SerialBT.begin("ESP32-Car");   // Nom du module Bluetooth (affiché sur ton téléphone)
  Serial.println("Bluetooth prêt !");

  // Broches moteur en sortie
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Initialement arrêtés
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  ledcAttachPin(enA, 0); // configurer PWM
  ledcSetup(0, 1000, 8); // canal 0, 1kHz, 8-bit (virtual PMW generator)
  ledcWrite(0, 0);
}

void turbo()
{
  vitesse = vitesseTurbo;
  delay (5000);
  vitesse = vitesseNormale;
}
 
void avancer(bool turboMode) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  if (turboMode == true)
   turbo();
  ledcWrite(0, vitesse);
}

void reculer() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  int vitesse = turboMode ? vitesseTurbo : vitesseNormale;
  ledcWrite(0, vitesse);
}

void gauche() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  ledcWrite(0, vitesseNormale);
}

void droite() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  ledcWrite(0, vitesseNormale);
}

void stopMoteurs() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  ledcWrite(0, 0);
}

void loop() {
  while (SerialBT.available() > 0) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') break;
    cmd += c;
  }

  if (cmd != "") {
    cmd.trim();  // Nettoie \r \n
    Serial.print("Commande reçue : ");
    Serial.println(cmd);

    if (cmd == "avant") avancer(false);
   
    else if (cmd == "arriere") reculer();
   
    else if (cmd == "gauche") gauche();
   
    else if (cmd == "droite") droite();
   
    else if (cmd == "stop") stopMoteurs();
   
    else if (cmd == "turbo") {
      turboMode = true;
      Serial.println("Turbo activé !");
      avancer(turboMode);
    }

    cmd = "";
  }

  delay(50);
}
