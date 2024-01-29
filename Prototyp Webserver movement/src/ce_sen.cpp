#include <Arduino.h>
#include <AccelStepper.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <FS.h>
#include "SPIFFS.h"

#define HALFSTEP 8

const float SteppDegree = 11.32; // Schritte pro Grad
int maxstepperSpeed = 2000;
int stepperSpeed = 200;
int stepperBesch = 100;
int sensor = 15;

// Pinbelegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

enum MovementState { FORWARD, TURNING, AFTER_TURN, STOPPED };
MovementState currentState = FORWARD;

void setup(void) {
  delay(3000);

  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.setSpeed(stepperSpeed);
  stepper1.setAcceleration(stepperBesch);
  stepper1.setCurrentPosition(0);

  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.setSpeed(stepperSpeed);
  stepper2.setAcceleration(stepperBesch);
  stepper2.setCurrentPosition(0);

  pinMode(sensor, INPUT);

  // Starte mit Vorwärtsbewegung
  stepper1.moveTo(stepper1.currentPosition() + 2000); 
  stepper2.moveTo(stepper2.currentPosition() + 2000);
}

void turn() {
  // Stoppe Motoren vor der Drehung
  stepper1.stop();
  stepper2.stop();

  float degree = 180; // Winkel für 90 Grad Drehung
  float moveRev = degree * SteppDegree;
  stepper1.moveTo(stepper1.currentPosition() + moveRev);
  stepper2.moveTo(stepper2.currentPosition() - moveRev);
  currentState = TURNING;
}
void stop() {
  stepper1.moveTo(stepper1.currentPosition() - 600);
  stepper2.moveTo(stepper2.currentPosition() - 600);

  currentState = STOPPED;
}


void forwardAfterTurn() {
  stepper1.moveTo(stepper1.currentPosition() + 2000); 
  stepper2.moveTo(stepper2.currentPosition() + 2000);
  currentState = AFTER_TURN;
}

void loop(void) {
  int sensorValue = digitalRead(sensor);

  stepper1.run();
  stepper2.run();

  // Prüfen, ob eine Aktion erforderlich ist
  if (!stepper1.isRunning() && !stepper2.isRunning()) {
    switch (currentState) {
      case FORWARD:
        if (sensorValue == 1) { // Sensor ausgelöst
          stop();
        }
        break;
      case STOPPED:
            turn();
        break;
      case TURNING:
        forwardAfterTurn();
        break;
      case AFTER_TURN:
        // Hier könnte eine weitere Aktion oder Bewegung implementiert werden
        break;
    }
  }
}
