#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

String cmd = "";

// Moteur 1
int enA1 = 14;
int in11 = 27;
int in21 = 26;

// Moteur 2
int in31 = 25;
int in41 = 23;
int enB1 = 32;

// Moteur 4
int enA2 = 21;
int in12 = 19;
int in22 = 18;

// Moteur 3
int in32 = 5;
int in42 = 17;
int enB2 = 16;

// Vitesse normale et turbo
bool turboMode = false;
int vitesseNormale = 191;
int vitesseTurbo = 255;

void setup() {
  
  Serial.begin(115200);          // Moniteur série
  SerialBT.begin("ESP32-Car");   // Nom du module Bluetooth (affiché sur ton téléphone)
  Serial.println("Bluetooth prêt !");

  // Broches moteur en sortie
  //Driver 1
  pinMode(enA1, OUTPUT);
  pinMode(in11, OUTPUT);
  pinMode(in21, OUTPUT);
  pinMode(in31, OUTPUT);
  pinMode(in41, OUTPUT);
  pinMode(enB1, OUTPUT);

  //Driver 2
  pinMode(enA2, OUTPUT);
  pinMode(in12, OUTPUT);
  pinMode(in22, OUTPUT);
  pinMode(in32, OUTPUT);
  pinMode(in42, OUTPUT);
  pinMode(enB2, OUTPUT);

  // Initialement arrêtés
  //Driver 1
  digitalWrite(in11, LOW);
  digitalWrite(in21, LOW);
  digitalWrite(in31, LOW);
  digitalWrite(in41, LOW);
  
  //Driver 2
  digitalWrite(in12, LOW);
  digitalWrite(in22, LOW);
  digitalWrite(in32, LOW);
  digitalWrite(in42, LOW);

  // configurer PWM
  ledcAttachPin(enA1, 1); 
  ledcSetup(1, 1000, 8); // canal 0, 1kHz, 8-bit (virtual PMW generator)
  ledcWrite(1, 0);

  ledcAttachPin(enB1, 2);
  ledcSetup(2, 1000, 8);
  ledcWrite(2, 0);

  ledcAttachPin(enA2, 3);
  ledcSetup(3, 1000, 8);
  ledcWrite(3, 0);

  ledcAttachPin(enB2, 4);
  ledcSetup(4, 1000, 8);
  ledcWrite(4, 0);

}

void stopCar() {
  // Arrête tous les moteurs
  digitalWrite(in11, LOW);
  digitalWrite(in21, LOW);
  digitalWrite(in31, LOW);
  digitalWrite(in41, LOW);

  digitalWrite(in12, LOW);
  digitalWrite(in22, LOW);
  digitalWrite(in32, LOW);
  digitalWrite(in42, LOW);

  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
  ledcWrite(4, 0);
}

void avancer() {
  // Driver 1
  digitalWrite(in11, HIGH);
  digitalWrite(in21, LOW);
  digitalWrite(in31, HIGH);
  digitalWrite(in41, LOW);

  // Driver 2
  digitalWrite(in12, HIGH);
  digitalWrite(in22, LOW);
  digitalWrite(in32, HIGH);
  digitalWrite(in42, LOW);

  ledcWrite(1, vitesseNormale);
  ledcWrite(2, vitesseNormale);
  ledcWrite(3, vitesseNormale);
  ledcWrite(4, vitesseNormale);
}

void reculer() {
  // Driver 1
  digitalWrite(in11, LOW);
  digitalWrite(in21, HIGH);
  digitalWrite(in31, LOW);
  digitalWrite(in41, HIGH);

  // Driver 2
  digitalWrite(in12, LOW);
  digitalWrite(in22, HIGH);
  digitalWrite(in32, LOW);
  digitalWrite(in42, HIGH);

  ledcWrite(1, vitesseNormale);
  ledcWrite(2, vitesseNormale);
  ledcWrite(3, vitesseNormale);
  ledcWrite(4, vitesseNormale);
}

void gauche() {
  // Moteurs gauche reculent, 
  digitalWrite(in11, LOW);
  digitalWrite(in21, HIGH);
  digitalWrite(in32, LOW);
  digitalWrite(in42, HIGH);

  //moteurs droite avancent
  digitalWrite(in12, HIGH);
  digitalWrite(in22, LOW);
  digitalWrite(in31, HIGH);
  digitalWrite(in41, LOW);

  ledcWrite(1, vitesseNormale);
  ledcWrite(2, vitesseNormale);
  ledcWrite(3, vitesseNormale);
  ledcWrite(4, vitesseNormale);
}

void droite() {
  // Moteurs gauche avancent
  digitalWrite(in11, HIGH);
  digitalWrite(in21, LOW);
  digitalWrite(in32, HIGH);
  digitalWrite(in42, LOW);

  //moteurs droite reculent
  digitalWrite(in12, LOW);
  digitalWrite(in22, HIGH);
  digitalWrite(in31, LOW);
  digitalWrite(in41, HIGH);

  ledcWrite(1, vitesseNormale);
  ledcWrite(2, vitesseNormale);
  ledcWrite(3, vitesseNormale);
  ledcWrite(4, vitesseNormale);
}


void loop() {


reculer();

  delay(50);
}
