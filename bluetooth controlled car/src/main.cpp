#include <Arduino.h>
#include <Bluepad32.h>

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

ControllerPtr myController;

// =====================
// === BLUETOOTH ===
// =====================
void onConnectedController(ControllerPtr ctl) {
  Serial.println("🎮 Contrôleur connecté !");
  myController = ctl;
}

void onDisconnectedController(ControllerPtr ctl) {
  Serial.println("❌ Contrôleur déconnecté !");
  myController = nullptr;
}

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

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();

  // Pins
  pinMode(in11, OUTPUT); pinMode(in21, OUTPUT);
  pinMode(in31, OUTPUT); pinMode(in41, OUTPUT);
  pinMode(in12, OUTPUT); pinMode(in22, OUTPUT);
  pinMode(in32, OUTPUT); pinMode(in42, OUTPUT);

  // PWM Setup (IMPORTANT: setup avant attach)
  ledcSetup(1, 1000, 8);
  ledcSetup(2, 1000, 8);
  ledcSetup(3, 1000, 8);
  ledcSetup(4, 1000, 8);

  ledcAttachPin(enA1, 1);
  ledcAttachPin(enB1, 2);
  ledcAttachPin(enA2, 3);
  ledcAttachPin(enB2, 4);

  stopCar();

  Serial.println("🚗 Prêt à se connecter à une manette Xbox !");
}

// =====================
// === LOOP ===
// =====================
void loop() {
  BP32.update();

  if (myController && myController->isConnected()) {

    int ly = myController->axisY();  // -512 à 512
    int lx = myController->axisX();  // -512 à 512

    bool boutonA = myController->a();
    turboMode = boutonA;

    int maxSpeed = turboMode ? vitesseTurbo : vitesseNormale;

    // ===== Dead Zone =====
    if (abs(ly) < 40) ly = 0;
    if (abs(lx) < 40) lx = 0;

    // ===== Mixage différentiel =====
    int left = ly + lx;
    int right = ly - lx;

    // Limiter plage joystick
    left = constrain(left, -512, 512);
    right = constrain(right, -512, 512);

    // Conversion vers PWM
    left = map(left, -512, 512, -maxSpeed, maxSpeed);
    right = map(right, -512, 512, -maxSpeed, maxSpeed);

    // Debug optionnel
    /*
    Serial.print("Left: ");
    Serial.print(left);
    Serial.print(" | Right: ");
    Serial.println(right);
    */

    setLeftMotors(left);
    setRightMotors(right);
  }
  else {
    stopCar();
  }

  delay(10);
}
