/*
Hilfe:
https://cool-web.de/arduino/multi-function-shield-step-motor.htm
https://www.airspayce.com/mikem/arduino/AccelStepper/
*/

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


//pin belegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

boolean turn1 = true; //track ob wir gerade fahren oder in einer Kurve
boolean turn2 = true;

void setup(void)
{
  delay(3000);

  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.move(1);
  stepper1.setAcceleration(stepperBesch);
  stepper1.setSpeed(stepperSpeed);
  stepper1.setCurrentPosition(0);

  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.move(-1);
  stepper2.setAcceleration(stepperBesch);
  stepper2.setSpeed(stepperSpeed);
  stepper2.setCurrentPosition(0);
}

void turnRight()
{
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.moveTo(moveRev);
  stepper1.run();
  stepper2.moveTo(-moveRev);
  stepper2.run();
}

void turnleft()
{
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.moveTo(-moveRev);
  stepper1.run();
  stepper2.moveTo(moveRev);
  stepper2.run();
}

void turnForword()
{
  stepper1.moveTo(10000);
  stepper1.run();
  stepper2.moveTo(10000);
  stepper2.run();
}

void stop()
{
  stepper1.stop();
  stepper2.stop();
}


void loop(void)
{

int sensorValue = digitalRead(sensor);
  if(sensorValue == 0)
  {
    turnForword();
  }
  if(sensorValue == 1)
  {
    if(turn1)
    {
      stop();
      delay(100);
    }
    else if(turn2)
    {
      float degree = 180;
      float moveRev = degree * SteppDegree;
      stepper1.moveTo(-moveRev);
      stepper1.run();
      stepper2.moveTo(moveRev);
      stepper2.run();
    }
  
    // toggeln 
    if(turn1)
    {
      turn1 = false;
      turn2 = true;
    }
    else if(turn2)
    {
      turn2 = false;
    }

  }
}

