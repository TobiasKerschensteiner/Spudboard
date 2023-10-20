/*
Hilfe:
https://cool-web.de/arduino/multi-function-shield-step-motor.htm
https://www.airspayce.com/mikem/arduino/AccelStepper/
AHHHHHHHHHHHHHHH
*/
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

//pin belegung
AccelStepper stepper1(AccelStepper::FULL4WIRE, 17, 18, 5, 16); //(Wenn + rück wenn - vor)
AccelStepper stepper2(AccelStepper::FULL4WIRE, 26, 14, 27, 12); //(Wenn + rück wenn - vor)

MultiStepper steppers;

void setup() {
  Serial.begin(9600);

  // Max Geschwindigkeit soll nicht mehr als 2000 betragen
  stepper1.setMaxSpeed(200);
  stepper2.setMaxSpeed(200);

  //sanfter anlauf
  stepper1.setAcceleration(100);
  stepper2.setAcceleration(100);

  //Stepper zusammen fügen. Geht bis zu Max 10
  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2);

}


//Vorwärtsfahren
void moveForward()
{
    int steps = 200; // Anzahl der schritte
    stepper1.move(-steps);
    stepper2.move(-steps);
    stepper1.run();
    stepper2.run();
}


//Rechtedrehung 90°
void turnRight()
{
    int steps = 200; // Anzahl der Schritte um 90° nach rechts zu drehen
    stepper1.move(steps);
    stepper2.move(-steps);
    stepper1.run();
    stepper2.run();
}

//Linksdrehung 90°
void turnLeft()
{

    int steps = 200; //Anzahl der Schritte um 90° nach links zu drehen 
    stepper1.move(-steps);
    stepper2.move(steps);
    stepper1.run();
    stepper2.run();
}

//Rückwärtsfahren falls von nöten
void moveBackward()
{
    int steps = 50000; //Anzahl der Schritte
    stepper1.move(steps);
    stepper2.move(steps);
    stepper1.run();
    stepper2.run();
}


//Test im kreis fahren
void loop()
{

moveForward();

 

}