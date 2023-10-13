
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

//pin belegung
AccelStepper stepper1(AccelStepper::FULL4WIRE, 2, 3, 4, 5);
AccelStepper stepper2(AccelStepper::FULL4WIRE, 8, 9, 10, 11);


MultiStepper steppers;

void setup() {
  Serial.begin(9600);

  // Max Geschwindigkeit soll nicht mehr als 2000 betragen
  stepper1.setMaxSpeed(200);
  stepper2.setMaxSpeed(200);

  //Stepper zusammen fügen. Geht bis zu Max 10
  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2);
}

//Vorwärtsfahren
void moveForward()
{
    int steps = 1000; // Anzahl der schritte
    stepper1.move(steps);
    stepper2.move(steps);
    steppers.run();
}

//Rechtedrehung 90°
void turnRight()
{
    int steps = 200; // Anzahl der Schritte um 90° nach rechts zu drehen
    stepper1.move(steps);
    stepper2.move(-steps);
    steppers.run();
}

//Linksdrehung 90°
void turnLeft()
{
    int steps = 200; //Anzahl der Schritte um 90° nach links zu drehen 
    stepper1.move(-steps);
    stepper2.move(steps);
    steppers.run();
}

//Rückwärtsfahren falls von nöten
void moveBackward()
{
    int steps = 1000; //Anzahl der Schritte
    stepper1.move(-steps);
    stepper2.move(-steps);
    steppers.run();
}

//Test im kreis fahren
void loop()
{
   for (int i = 0; i < 4; i++)
   {
    moveForward();
    delay(1000); //1 sek warten
    turnRight();
    delay(1000); //1 sek warten

    moveForward();
    delay(1000); //1 sek warten
    turnLeft();
    delay(1000);
   }
}