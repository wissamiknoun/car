#include <Arduino.h>
#include <Bluepad32.h>

// === MOTEURS ===

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

// === PARAMÈTRES ===
bool turboMode = false;
int vitesseNormale = 125;
int vitesseTurbo = 255;

// === Bluepad32 ===
ControllerPtr myController;

void onConnectedController(ControllerPtr ctl) {
  Serial.println("🎮 Contrôleur connecté !");
  myController = ctl;
}

void onDisconnectedController(ControllerPtr ctl) {
  Serial.println("❌ Contrôleur déconnecté !");
  myController = nullptr;
}

// === MOTEUR FONCTIONS ===
void stopCar() {
  // Arrête tous les moteurs
  digitalWrite(in11, LOW); digitalWrite(in21, LOW);
  digitalWrite(in31, LOW); digitalWrite(in41, LOW);
  digitalWrite(in12, LOW); digitalWrite(in22, LOW);
  digitalWrite(in32, LOW); digitalWrite(in42, LOW);

  ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0); ledcWrite(4, 0);
}

void avancer(int vitesse) {
  // Driver 1
  digitalWrite(in11, HIGH); digitalWrite(in21, LOW);
  digitalWrite(in31, HIGH); digitalWrite(in41, LOW);
  // Driver 2
  digitalWrite(in12, HIGH); digitalWrite(in22, LOW);
  digitalWrite(in32, HIGH); digitalWrite(in42, LOW);
  ledcWrite(1, vitesse); ledcWrite(2, vitesse); ledcWrite(3, vitesse); ledcWrite(4, vitesse);
}

void reculer(int vitesse) {
  // Driver 1
  digitalWrite(in11, LOW); digitalWrite(in21, HIGH);
  digitalWrite(in31, LOW); digitalWrite(in41, HIGH);
  // Driver 2
  digitalWrite(in12, LOW); digitalWrite(in22, HIGH);
  digitalWrite(in32, LOW); digitalWrite(in42, HIGH);
  ledcWrite(1, vitesse); ledcWrite(2, vitesse); ledcWrite(3, vitesse); ledcWrite(4, vitesse);
}

void gauche(int vitesse) {
  // Moteurs gauche reculent
  digitalWrite(in11, LOW); digitalWrite(in21, HIGH);
  digitalWrite(in32, LOW); digitalWrite(in42, HIGH);
  // Moteurs droite avancent
  digitalWrite(in12, HIGH); digitalWrite(in22, LOW);
  digitalWrite(in31, HIGH); digitalWrite(in41, LOW);
  ledcWrite(1, vitesse); ledcWrite(2, vitesse); ledcWrite(3, vitesse); ledcWrite(4, vitesse);
}

void droite(int vitesse) {
  // Moteurs gauche avancent
  digitalWrite(in11, HIGH); digitalWrite(in21, LOW);
  digitalWrite(in32, HIGH); digitalWrite(in42, LOW);
  // Moteurs droite reculent
  digitalWrite(in12, LOW); digitalWrite(in22, HIGH);
  digitalWrite(in31, LOW); digitalWrite(in41, HIGH);
  ledcWrite(1, vitesse); ledcWrite(2, vitesse); ledcWrite(3, vitesse); ledcWrite(4, vitesse);
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys(); // Efface anciens appairages

  // Configurer les pins moteurs
  pinMode(enA1, OUTPUT); pinMode(in11, OUTPUT); pinMode(in21, OUTPUT);
  pinMode(enB1, OUTPUT); pinMode(in31, OUTPUT); pinMode(in41, OUTPUT);
  pinMode(enA2, OUTPUT); pinMode(in12, OUTPUT); pinMode(in22, OUTPUT);
  pinMode(enB2, OUTPUT); pinMode(in32, OUTPUT); pinMode(in42, OUTPUT);

  // PWM
  ledcAttachPin(enA1, 1); ledcSetup(1, 1000, 8);
  ledcAttachPin(enB1, 2); ledcSetup(2, 1000, 8);
  ledcAttachPin(enA2, 3); ledcSetup(3, 1000, 8);
  ledcAttachPin(enB2, 4); ledcSetup(4, 1000, 8);

  stopCar();
  Serial.println("Prêt à se connecter à une manette Xbox !");
}

// === LOOP ===
void loop() {
  BP32.update();

  if (myController && myController->isConnected()) {
    int ly = myController->axisY();  // Joystick gauche vertical
    int lx = myController->axisX();  // Joystick gauche horizontal

    bool boutonA = myController->a(); // Bouton A active turbo

    // Gestion turbo
    turboMode = boutonA;
    int vitesse = turboMode ? vitesseTurbo : vitesseNormale;

    // Déplacement selon joystick
    if (ly < -10000) avancer(vitesse);
    else if (ly > 10000) reculer(vitesse);
    else if (lx < -10000) gauche(vitesse);
    else if (lx > 10000) droite(vitesse);
    else stopCar();
  }

  delay(20);
}
