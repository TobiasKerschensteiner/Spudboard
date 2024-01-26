#include <Arduino.h>
#include <AccelStepper.h>

#define FULLSTEP 4
#define HALFSTEP 8

const float SteppDegree = 11.32;
int maxstepperSpeed = 2000;
int stepperSpeed = 200;
int stepperBesch = 100;
int sensor = 15;

// Pinbelegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

enum MovementState { FORWARD, TURNING, STOPPED };
MovementState currentState = STOPPED;

void setup(void) {
  delay(3000);

  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.setAcceleration(stepperBesch);
  stepper1.setSpeed(stepperSpeed);
  stepper1.setCurrentPosition(0);

  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.setAcceleration(stepperBesch);
  stepper2.setSpeed(stepperSpeed);
  stepper2.setCurrentPosition(0);

  pinMode(sensor, INPUT);
}

void turn(int direction) {
  // Richtung: 1 für Rechtsdrehung, -1 für Linksdrehung
  if (currentState != TURNING) {
    float degree = 90; // Winkel für Drehung
    float moveRev = degree * SteppDegree;
    stepper1.moveTo(stepper1.currentPosition() + direction * moveRev);
    stepper2.moveTo(stepper2.currentPosition() - direction * moveRev);
    currentState = TURNING;
  }
}

void forward() {
  if (currentState != FORWARD) {
    stepper1.moveTo(stepper1.currentPosition() + 2000); // Wert für Vorwärtsbewegung
    stepper2.moveTo(stepper2.currentPosition() + 2000);
    currentState = FORWARD;
  }
}

void stopMotors() {
  if (currentState != STOPPED) {
    stepper1.stop();
    stepper2.stop();
    currentState = STOPPED;
  }
}

void loop(void) {
  int sensorValue = digitalRead(sensor);

  stepper1.run();
  stepper2.run();

  if (sensorValue == 0 && !stepper1.isRunning() && !stepper2.isRunning()) {
    forward();
  } else if (sensorValue == 1) {
    if (currentState == FORWARD) {
      stopMotors();
      delay(100);
      turn(1); // Ändern Sie den Wert auf -1 für eine Linksdrehung
    }
  }
}
