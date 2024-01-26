#include <Arduino.h>
#include <AccelStepper.h>

#define FULLSTEP 4
#define HALFSTEP 8

const short fullRevolution = 2048;
const float SteppDegree = 11.32;
int maxstepperSpeed = 2000;
int stepperSpeed = 200;
int stepperBesch = 100;
int sensor = 15;

// Pinbelegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

enum MovementState { FORWARD, TURN_RIGHT, TURN_LEFT, STOPPED };
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

void turnRight() {
  if (currentState != TURN_RIGHT) {
    float degree = 180;
    float moveRev = degree * SteppDegree;
    stepper1.moveTo(stepper1.currentPosition() + moveRev);
    stepper2.moveTo(stepper2.currentPosition() - moveRev);
    currentState = TURN_RIGHT;
  }
}

void turnLeft() {
  if (currentState != TURN_LEFT) {
    float degree = 180;
    float moveRev = degree * SteppDegree;
    stepper1.moveTo(stepper1.currentPosition() - moveRev);
    stepper2.moveTo(stepper2.currentPosition() + moveRev);
    currentState = TURN_LEFT;
  }
}

void turnForward() {
  if (currentState != FORWARD) {
    stepper1.moveTo(stepper1.currentPosition() + 10000);
    stepper2.moveTo(stepper2.currentPosition() + 10000);
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
    turnForward();
  } else if (sensorValue == 1) {
    if (currentState == FORWARD) {
      stopMotors();
      delay(100);
      turnRight(); // Sie können dies in turnLeft() ändern, je nach Bedarf
    }
  }
}
