/*
Hilfe:
https://cool-web.de/arduino/multi-function-shield-step-motor.htm
https://www.airspayce.com/mikem/arduino/AccelStepper/
*/


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <FS.h>
#include "SPIFFS.h"
#include <Arduino.h>
#include <AccelStepper.h>

#define HALFSTEP 8

// Konstanten für die Drehung
const float stepsPerRevolution = 2048; // Schritte für eine volle Umdrehung im Half-Step-Modus
const float SteppDegree = 11.32;
const int maxSpeed = 2000;
const int maxspeeddre = 1000;
const int besch = 200;
const int desiredSpeed = 1000; // Gewünschte Geschwindigkeit in Schritten pro Sekunde
const int sensorPin = 15; // Pin-Nummer des Sensors
const int delays = 2000;
const int dist = 1000;

// Pin-Belegung für beide Stepper-Motoren
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

enum State {MOVING_FORWARD, TURNING_RIGHT, MOVING_SHORT_DISTANCER, TURNING_LEFT, MOVING_SHORT_DISTANCEL, TURNING_LEFT2, TURNING_RIGHT2, STOPPING };
State currentState = MOVING_FORWARD;
bool hasTurnedRight = false;
bool turnLeftNext = false; // Flag, um zu bestimmen, ob als nächstes nach links gedreht werden soll
//bool turnRightNext = false;

void setup() {
  // Initialisiere die Motoren mit der gewünschten Geschwindigkeit
  stepper1.setMaxSpeed(maxSpeed);
  stepper1.setSpeed(desiredSpeed);

  stepper2.setMaxSpeed(maxSpeed);
  stepper2.setSpeed(desiredSpeed);

  pinMode(sensorPin, INPUT); // Setze den Sensor-Pin als Eingang
}


void turnRight()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.move(moveRev);
  stepper2.move(-moveRev);

    // Führe die Drehung aus und warte, bis sie abgeschlossen ist
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}


void turnLeft()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.move(-moveRev);
  stepper2.move(moveRev);

    // Führe die Drehung aus und warte, bis sie abgeschlossen ist
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}


void moveForward() {
  stepper1.setSpeed(desiredSpeed);
  stepper2.setSpeed(desiredSpeed);
  stepper1.runSpeed();
  stepper2.runSpeed();
  
}

void moveShortDistance(int steps) {
  stepper1.move(steps);
  stepper2.move(steps);
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

void moveShortDistanceL(int steps) {
  stepper1.move(steps);
  stepper2.move(steps);
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

void stopMotors() {
  stepper1.stop(); // Stoppe Stepper 1
  stepper2.stop(); // Stoppe Stepper 2
  while (stepper1.isRunning() || stepper2.isRunning()) {
    // Warte, bis beide Motoren vollständig angehalten haben
    stepper1.run();
    stepper2.run();
  }
}

void loop() {
  int sensorValue = digitalRead(sensorPin); // Lese den Sensorwert

  switch (currentState) {
    case MOVING_FORWARD:
      moveForward(); // Bewege dich vorwärts
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
        if (turnLeftNext) {
          currentState = TURNING_LEFT; // Wechsle den Zustand zu Linksabbiegung
          turnLeftNext = false; // Setze zurück, damit das nächste Abbiegen wieder rechts ist
        } else {
          currentState = TURNING_RIGHT; // Wechsle den Zustand zu Rechtsabbiegung
          //turnRightNext = false;
        }
      }
      break;

    case TURNING_RIGHT:
      turnRight(); // Führe eine Rechtsabbiegung aus
      currentState = MOVING_SHORT_DISTANCER;
      break;

    case MOVING_SHORT_DISTANCER:
      moveShortDistance(dist); // Bewege dich eine kurze Strecke vorwärts
      currentState = TURNING_RIGHT2;
      break;

    case TURNING_RIGHT2:
      turnRight(); // Führe die zweite Rechtsabbiegung aus
      currentState = MOVING_FORWARD;
      turnLeftNext = true; // Setze das Flag, damit beim nächsten Sensor HIGH links abgebogen wird
      break;

    case TURNING_LEFT:
      turnLeft(); // Führe eine Linksabbiegung aus
      currentState = MOVING_SHORT_DISTANCEL; // Kehre zurück zum Zustand vorwärts bewegen
      break;

    case MOVING_SHORT_DISTANCEL:
      moveShortDistanceL(dist);
      currentState = TURNING_LEFT2;
      break;

    case TURNING_LEFT2:
      turnLeft();
      currentState = MOVING_FORWARD;
      //turnRightNext = true;
      break;
  }
}